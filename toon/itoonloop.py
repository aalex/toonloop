#!/usr/bin/env python
#
# ToonLoop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# <alexandre@quessy.net> & <le.businessman@gmail.com>
# http://www.toonloop.com
#
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# ToonLoop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ToonLoop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with ToonLoop.  If not, see <http://www.gnu.org/licenses/>.
#

"""
ToonLoop is a realtime stop motion performance tool. 

The idea is to spread its use for teaching new medias to children and to 
give a professional tool for movie creators.

In the left window, you can see what is seen by the live camera.
In the right window, it is the result of the stop motion loop.

Usage :
 - Press the SPACE bar to grab a frame.
 - Press DELETE or BACKSPACE to delete the last frame.
 - Press 'r' to reset and start the current sequence. (an remove all its frames)
 - Press 's' to save the current sequence as a QuickTime movie.
 - Press 'i' to print current loop frame number, number of frames in loop 
   and global framerate.
 - Press 'h' to print a help message.
 - Press UP to increase frame rate.
 - Press DOWN to decrease frame rate.
 - Press 'a' to toggle on/off the auto recording. 
   (it records one frame on every frame) It is an intervalometer.
   Best used to create timelapse sequences automatically.
 - Press 'k' or 'j' to increase or decrease the auto rate.

INSTALLATION NOTES : 
The camera module for pygame is available from pygame's svn revision 
1744 or greater
svn co svn://seul.org/svn/pygame/trunk

The startup file to execute is toonloop
"""
 
import sys
from time import strftime
import os
import shutil
import glob
import pprint

from twisted.internet import reactor

from rats import render
from rats.serialize import Serializable
from rats.observer import Subject

import toon
from toon import opensoundcontrol
from toon import mencoder
from toon.draw import texture_from_image
from toon.draw import draw_textured_square
from toon import puredata
from toon import chromakey
try:
    from toon import web_server
except ImportError, e:
    print "For web support, please install the python-nevow package."
    print e.message

import pygame
import pygame.camera
from pygame.locals import *
from pygame import time
from OpenGL.GL import *

__version__ = "1.0 beta"

class ToonLoopError(Exception):
    """
    Any error ToonLoop might encouter
    """
    pass

class ToonShot(Serializable):
    """
    ToonLoop shot.
    A shot is a serie of frames. 
    """
    def __init__(self, id, **argd):
        """
        :param id: int 
        """
        self.id = id
        self.playhead = 0
        self.playhead_iterate_every = 3 # Ratio that determines the framerate
        #self.framerate = 12
        self.__dict__.update(argd)
        self.images = []
        # to do:
        #self.writehead = 0
        #self.images_file_names = []
        #self._intervalometer_delayed_id = None
        #self.intervalometer_enabled = False
        #self.intervalometer_rate_seconds = 0.1

# class ToonProject(object):
#     """
#     Project file with shots and sequences.
# 
#     Serializes project and save image files.
#     """
#     def __init__(self, name, **kwargs):
#         self.path = os.path.expanduser("~/toonloop")
#         self.name = "toonloop"
#         self.datetime = self.now()
#         self.__dict__.update(argd)
#         # self.shots = []
# 
#     def now(self):
#         return strftime("%Y-%m-%d_%Hh%Mm%S")
#         
#     def __str__(self):
#         """
#         Full path of the project folder.
#         """
#         return "%s/%s_%s" % (self.path, self.name, self.datetime)
        
class Api(Subject):
    """
    The public API for ToonLoop user interfaces.
    """
    def __init__(self, app):
        self.app = app

    def print_help(self):
        """
        Prints help and usage.
        """
        print "ToonLoop keyboard controls : "
        print "<Space bar> = add image to loop "
        print "<Backspace> = remove image from loop "
        print "r = reset loop"
        print "p = pause"
        print "o = toggle onion skin on/off"
        print "i = print current loop frame number, number of frames in loop and global framerate"
        print "h = print this help message"
        print "s = saves all images as jpeg and movie"
        print "a = enable the intervalometer auto grab"
        print "k = increase the intervalometer interval"
        print "j = decrease the intervalometer interval"
        print "[1, 9]  = select the current shot number"
        print "<Esc> = quit program\n"

    def print_stats(self):
        """
        Print statistics
        """
        try:
            print "Current playhead: " + str(self.app.shot.playhead)
            print "Num images: " + str(len(self.app.shot.images))
            print "FPS: %d" % (self.app.fps)
            print "Playhead frequency ratio: 30 / %d" % (self.app.shot.playhead_iterate_every)
            pprint.pprint(self.app.config.__dict__)
            print 'pygame.display.Info(): ', pygame.display.Info()
        except AttributeError, e:
            print sys.exc_info()


class ToonLoop(render.Game):
    """
    ToonLoop is a realtime stop motion tool.
    """
    TEXTURE_MOST_RECENT = 0
    TEXTURE_PLAYBACK = 1
    TEXTURE_ONION = 2

    def __init__(self, config):
        """
        Startup poutine.
        """
        self.config = config
        self.api = Api(self)
        # size of the rendering window
        self._display_size = (self.config.display_width, self.config.display_height)
        self.running = True
        self.pd = None
        self.paused = False
        self.image_size = (self.config.image_width, self.config.image_height)
        self.clock = pygame.time.Clock()
        self.fps = 0 # for statistics
        #self.images_list = [] # only editing one shot for now
        #self.shot.playhead = 0 
        self.shot_id = 0 # Currently selected shot
        self.shot = None # current ToonShot instance
        self.shots = [] # ToonShot instances
        self._init_shots()
        self.renderer = None # Renderer instance that owns it.
        self._intervalometer_delayed_id = None
        # the icon
        package_base = os.path.dirname(toon.__file__)
        icon = pygame.image.load(os.path.join(package_base, 'data', "icon.png"))
        pygame.display.set_icon(icon) # a 32 x 32 surface
        # the pygame window
        self.display = pygame.display.set_mode(self._display_size, OPENGL | DOUBLEBUF | HWSURFACE)
        pygame.display.set_caption("ToonLoop")
        pygame.mouse.set_visible(False)
        # the images
        self.most_recent_image = pygame.surface.Surface(self.image_size) # , 0, self.display)
        
        self.osc = None
        # self.fudi = None
        self.camera = None
        self.is_mac = False
        self.textures = [0, 0, 0] # list of OpenGL texture objects id
        self._setup_camera()
        self._setup_window()
        self._clear_playback_view()
        self._clear_onion_peal()
        self._playhead_iterator = 0
        # copy conf elements
        self.intervalometer_on = self.config.intervalometer_on
        self.intervalometer_rate_seconds = self.config.intervalometer_rate_seconds
        if self.config.verbose:
            pprint.pprint(self.config.__dict__)
        if config.intervalometer_on:
            self.intervalometer_toggle(True)
        reactor.callLater(0, self._setup_services)

    def _init_shots(self):
        """
        Creates a new project.
        """
        for i in range(self.config.max_num_shots):
            self.shots.append(ToonShot(i))
        try:
            self.shot = self.shots[self.shot_id]
        except IndexError:
            self.shot_id = 0
            self.shot = self.shots[self.shot_id]

    def shot_select(self, index=0):
        """
        Selects an other shot
        """
        self.shot_id = index
        self.shot = self.shots[index]
        if self.config.verbose:
            print "Shot #%s"  %(self.shot_id)
        self._clear_playback_view()
        
    def _setup_window(self):
        """
        OpenGL setup.
        """
        # create OpenGL texture objects 
        # window is 1280 x 960
        self._resize_window(self._display_size)
        glEnable(GL_TEXTURE_RECTANGLE_ARB) # 2D)
        glEnable(GL_BLEND)
        glShadeModel(GL_SMOOTH)
        glClearColor(0.0, 0.0, 0.0, 0.0) # black background
        glColor4f(1.0, 1.0, 1.0, 1.0) # self.config.playback_opacity) # for now we use it for all
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        for i in range(len(self.textures)):
            self.textures[i] = glGenTextures(1)
        if self.config.chromakey_enabled: 
            chromakey.program_init()
            chromakey.config = self.config.__dict__
        # print "texture names : ", self.textures

    def _resize_window(self, (width, height)):
        """
        Called when we resize the window.
        (fullscreen on/off)

        The OpenGL coordinates go from -4 to 4 horizontally
        and -3 to 3 vertically.
        (ratio is 4:3)
        """
        print "resize", width, height
        if height == 0:
            height = 1
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(-4.0, 4.0, -3.0, 3.0, -1.0, 1.0)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def _setup_camera(self):
        """
        Starts using the video camera.
        """
        if os.uname()[0] == 'Darwin':
            self.is_mac = True
        else:
            self.is_mac = False
        size = (self.config.image_width, self.config.image_height)
        try:
            pygame.camera.init() 
        # except AttributeError, e:
        #     print "Sometimes the camera module need it's init()",
        #     print " to be called and sometimes not."
        except Exception, e:
            print "error calling pygame.camera.init()", e.message
            print sys.exc_info()
            raise ToonLoopError("Error initializing the video camera. %s" % (e.message))
        try:
            print "cameras :", pygame.camera.list_cameras()
            if self.is_mac:
                print "Using camera %s" % (self.config.video_device)
                self.camera = pygame.camera.Camera(self.config.video_device, size)
            else:
                print "Using camera /dev/video%d" % (self.config.video_device)
                self.camera = pygame.camera.Camera("/dev/video%d" % (self.config.video_device), size)
            self.camera.start()
        except SystemError, e:
            print sys.exc_info()
            raise ToonLoopError("Invalid camera. %s" % (str(e.message)))
        except Exception, e:
            print sys.exc_info()
            raise ToonLoopError("Invalid camera. %s" % (str(e.message)))
        
    def _setup_services(self):
        """
        Starts the network services.

        Called once the Twisted reactor has been started. 
        """
        #self.osc = opensoundcontrol.ToonOsc(self)
        #index_file_path = os.path.join(os.curdir, 'toon', 'index.rst') 
        try:
            self.web = web_server.start(self, self.config.web_server_port,
                static_files_path=self.config.toonloop_home)
                #index_file_path=index_file_path)
        except:
            print "Error loading web UI :"
            print sys.exc_info()
        #try:
        self.pd = puredata.start(self) #, receive_port=15555, send_port=17777, send_host="localhost")
        #except:
        #    print "Error loading puredata:", sys.exc_info()

    def frame_add(self):
        """
        Copies the last grabbed frame to the list of images.
        """
        if self.is_mac:
            self.shot.images.append(self.most_recent_image.copy())
        else:
            self.shot.images.append(self.most_recent_image)
        # Creates an OpenGL texture
        texture_from_image(self.textures[self.TEXTURE_ONION], self.most_recent_image)
    
    def draw(self):
        """
        Renders one frame.
        Called from the event loop. (twisted)
        """
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        if not self.paused:
            self._playhead_iterator = (self._playhead_iterator + 1) % self.shot.playhead_iterate_every
            if self._playhead_iterator ==  0:
                self._playhead_iterate()
                # 30/3 = 10 FPS
        self._camera_grab_frame() # grab a frame
        
        # --------- edit view
        if self.config.chromakey_enabled and self.config.chromakey_on: 
            chromakey.program_enable()
            chromakey.set_program_uniforms()
        self._draw_edit_view()
        if self.config.chromakey_enabled and self.config.chromakey_on: 
            chromakey.program_disable()
        # ---------- onion skin
        if self.config.chromakey_enabled and self.config.chromakey_on: 
            chromakey.program_enable()
            chromakey.set_program_uniforms()
        self._draw_onion_skin()
        if self.config.chromakey_enabled and self.config.chromakey_on: 
            chromakey.program_disable()
        # ---------- playback view
        if self.config.chromakey_enabled and self.config.chromakey_on: 
            chromakey.program_enable()
            chromakey.set_program_uniforms()
        self._draw_playback_view()
        if self.config.chromakey_enabled and self.config.chromakey_on: 
            chromakey.program_disable()
        
        self.clock.tick()
        self.fps = self.clock.get_fps()
        pygame.display.flip()
        # old : pygame.display.update()

    def _camera_grab_frame(self):
        """
        Image capture from the video camera.
        """
        if self.is_mac:
            self.camera.get_image(self.most_recent_image)
        else:
            self.most_recent_image = self.camera.get_image()
        # Create an OpenGL texture
        texture_from_image(self.textures[self.TEXTURE_MOST_RECENT], self.most_recent_image)
        
    def _draw_edit_view(self):
        """
        Renders edit view (the live camera + onion peal)
        """
        glPushMatrix()
        glTranslatef(-2.0, 0.0, 0.0)
        glScalef(2.0, 1.5, 1.0)
        # most recent grabbed :
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.textures[self.TEXTURE_MOST_RECENT])
        draw_textured_square(self.config.image_width, self.config.image_height)
        # self.display_width = 1024
        glPopMatrix()
        # old: self.display.blit(self.most_recent_image, (0, 0))

    def _draw_onion_skin(self):
        # Onion skin :
        if self.config.onionskin_enabled:
            glPushMatrix()
            glTranslatef(-2.0, 0.0, 0.0)
            glScalef(2.0, 1.5, 1.0)
            glColor4f(1.0, 1.0, 1.0, self.config.onionskin_opacity)
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.textures[self.TEXTURE_ONION])
            draw_textured_square(self.config.image_width, self.config.image_height)
            glColor4f(1.0, 1.0, 1.0, 1.0) # self.config.playback_opacity)
            glPopMatrix()

    def _draw_playback_view(self):
        """
        Renders the playback view. 
        """
        glPushMatrix()
        glTranslatef(2.0, 0.0, 0.0)
        glScalef(2.0, 1.5, 1.0)
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.textures[self.TEXTURE_PLAYBACK])
        draw_textured_square(self.config.image_width, self.config.image_height)
        glPopMatrix()
    
    def _playhead_iterate(self):
        """ 
        Increments the playhead position of one frame
        """
        if self.shot.playhead < len(self.shot.images):
            image = self.shot.images[self.shot.playhead]
            texture_from_image(self.textures[self.TEXTURE_PLAYBACK], image)
            self.shot.playhead += 1
            # old: self.display.blit(self.shot.images[self.shot.playhead], (self.config.image_width, 0))
        else:
            self.shot.playhead = 0

    def _clear_playback_view(self):
        """
        Sets all pixels in the playback view as black.
        """
        blank_surface = pygame.Surface((self.config.image_width, self.config.image_height))
        texture_from_image(self.textures[self.TEXTURE_PLAYBACK], blank_surface)
        # old:
        # playback_pos = (self.config.image_width, 0)
        # self.display.blit(blank_surface, playback_pos)
        
    def _clear_onion_peal(self):
        """
        Sets all pixels in the onion peal as black.
        """
        blank_surface = pygame.Surface((self.config.image_width, self.config.image_height))
        texture_from_image(self.textures[self.TEXTURE_ONION], blank_surface)

    def pause(self, val=None):
        """
        Toggles on/off the pause
        """
        if val is not None:
            self.paused = val
        else:
            self.paused = not self.paused

    def shot_reset(self):
        """
        Deletes all frames from the current animation
        """
        self.shot.images = []
        self._clear_playback_view()

    def shot_save(self):
        """
        Saves all images as jpeg and encodes them to a MJPEG movie.
        
        See _write_next_image
        """
        # TODO : in a thread
        file_name = strftime("%Y-%m-%d_%Hh%Mm%S") # without an extension.
        path = os.path.join(self.config.toonloop_home, self.config.project_name)
        if self.config.verbose:
            print "Saving images ", path, file_name
        try:
            os.makedirs(path)
        except OSError, e:
            pass # print "error creating directory", path, e.message
        reactor.callLater(0, self._write_01_next_image, path, file_name, 0)

    def _write_01_next_image(self, path, file_name, index):
        """
        Saves each image using twisted in order not to freeze the app.
        Uses the JPG extension.
        """
        if index < len(self.shot.images):
            name = ("%s/%s_%5d.jpg" % (path, file_name, index)).replace(' ', '0')
            if self.config.verbose:
                print "writing %s" % (name)
            pygame.image.save(self.shot.images[index], name) # filename extension makes it a JPEG
            reactor.callLater(0, self._write_01_next_image, path, file_name, index + 1)
        else:
            reactor.callLater(0, self._write_02_images_done, path, file_name, index)
    
    def _write_02_images_done(self, path, file_name, index):
        """
        Converts the list of images in a motion-JPEG .mov video file.
        """
        if index > 0:
            if self.config.verbose:
                print "\nConverting to motion JPEG in Quicktime container."
            fps = 6 # TODO
            #self.shot.increment_every # self.shot.framerate
            #fps = self.renderer.desired_fps 

            deferred = mencoder.jpeg_to_movie(file_name, path, fps, self.config.verbose, self.config.image_width, self.config.image_height)
            deferred.addCallback(self._write_03_movie_done, file_name, path, index)
            # to do : serialize shots with file names
            # self.project_file = 'project.txt'

    def _write_03_movie_done(self, results, file_name, path, index):
        """
        Called when mencoder conversion is done.
        MOV file.
        """
        if self.config.verbose:
            print "Done converting %s/%s.mov" % (path, file_name)
        reactor.callLater(1.0, self._write_04_delete_images, path, file_name, index)


    def _write_04_delete_images(self, path, file_name, index):
        """
        deletes JPG images. 
        renames MOV file.
        """
        if self.config.delete_jpeg:
            files = glob.glob("%s/%s_*.jpg" % (path, file_name))
            for f in files:
                try:
                    os.remove(f)
                except OSError, e:
                    print "%s Error removing file %s" % (e.message, f)
        try:
            src = "%s/%s.mov" % (path, file_name)
            dest = "%s/clip_%s.mov" % (path, file_name)
            shutil.move(src, dest)
            if self.config.verbose:
                print 'renamed %s to %s' % (src, dest) 
        except IOError, e:
            print "%s Error moving file %s to %s" % (e.message, src, dest)

    def frame_remove(self):
        """
        Deletes the last frame from the current list of images.
        """
        if self.shot.images != []:
            self.shot.images.pop()
            # would it be better to also delete it ? calling del
            if self.shot.images == []:
                self._clear_playback_view()
            else:
                pass 
                #texture_from_image(self.textures[self.TEXTURE_ONION], self.most_recent_image)
    
    def framerate_increase(self, dir=1):
        """
        Increase or decreases the FPS
        :param dir: by how much increment it.
        """
        # TODO: for the user, it should be in FPS, not some weird ratio
        # does not alter the FPS of the renderer, but rather only the playback rate.
        # if self.renderer is not None:
        #     # accesses the Renderer instance that owns this.
        #     will_be = self.renderer.desired_fps + dir
        #     if will_be > 0 and will_be <= 60:
        #         self.renderer.desired_fps = will_be
        #         print "FPS:", will_be
        will_be = self.shot.playhead_iterate_every - dir
        if will_be > self.config.min_framerate and will_be <= self.config.max_framerate:
            self.shot.playhead_iterate_every = will_be
            if self.config.verbose:
                print "Playhead frequency ratio: 30 / %d" % (will_be)
    
    def process_events(self, events):
        """
        Processes pygame events.
        :param events: got them using pygame.event.get()
        """
        for e in events:
            if e.type == QUIT:
                self.running = False
            # TODO : catch window new size when resized.
            elif e.type == pygame.VIDEORESIZE:
                print "VIDEORESIZE", e
            elif e.type == KEYDOWN: 
                if e.key == K_k: # K
                    self.intervalometer_rate_increase(1)
                elif e.key == K_j: # J
                    self.intervalometer_rate_increase(-1)
                elif e.key == K_f: # F
                    pygame.display.toggle_fullscreen()
                elif e.key == K_i: # I
                    try:
                        self.api.print_stats()
                    except Exception, e:
                        print e.message
                elif e.key == K_p: # P
                    self.pause()
                elif e.key == K_r: # R 
                    self.shot_reset()
                elif e.key == K_h: # H
                    self.api.print_help()
                elif e.key == K_s: # S 
                    self.shot_save()
                elif e.key == K_o: # O
                    self.onionskin_toggle()
                elif e.key == K_a: # A
                    print "toggle intervalometer"
                    self.intervalometer_toggle()
                elif e.key == K_0: # 0 to 9
                    self.shot_select(0)
                elif e.key == K_1:
                    self.shot_select(1)
                elif e.key == K_2:
                    self.shot_select(2)
                elif e.key == K_3:
                    self.shot_select(3)
                elif e.key == K_4:
                    self.shot_select(4)
                elif e.key == K_5:
                    self.shot_select(5)
                elif e.key == K_6:
                    self.shot_select(6)
                elif e.key == K_7:
                    self.shot_select(7)
                elif e.key == K_8:
                    self.shot_select(8)
                elif e.key == K_9:
                    self.shot_select(9)
                elif e.key == K_UP: # UP
                    self.framerate_increase(1)
                elif e.key == K_DOWN: # DOWN
                    self.framerate_increase(-1)
                elif e.key == K_SPACE: # SPACE
                    self.frame_add()
                elif e.key == K_BACKSPACE: # BACKSPACE
                    self.frame_remove()
                elif e.key == K_ESCAPE: #  or e.key == K_q: # ESCAPE or Q
                    self.running = False

    def onionskin_toggle(self, val=None):
        """
        Toggles on/off the onion skin. 
        (see most recent frame grabbed in transparency)
        """
        if val is not None:
            self.config.onionskin_enabled = val
        else:
            self.config.onionskin_enabled = not self.config.onionskin_enabled
            
                
    def intervalometer_toggle(self, val=None):
        """
        Toggles on/off the intervalometer / timelapse / auto mode.
        """
        if self.config.intervalometer_enabled:
            if val is not None:
                self.intervalometer_on = val
            else:
                self.intervalometer_on = not self.intervalometer_on
            if self.intervalometer_on:
                self._intervalometer_delayed_id = reactor.callLater(0, self._intervalometer_frame_add)
                if self.config.verbose:
                    print "intervalometer ON"
            else:
                if self._intervalometer_delayed_id is not None:
                    if self._intervalometer_delayed_id.active():
                        self._intervalometer_delayed_id.cancel()
                        if self.config.verbose:
                            print "intervalometer OFF"
    
    def intervalometer_rate_increase(self, dir=1):
        """
        Increase or decreases the intervalometer rate. (in seconds)
        :param dir: by how much increment it.
        """
        if self.config.intervalometer_enabled:
            will_be = self.intervalometer_rate_seconds + dir
            if will_be > 0 and will_be <= 60:
                self.intervalometer_rate_seconds = will_be
                print "auto rate:", will_be

    def _intervalometer_frame_add(self):
        """
        Called when it is time to automatically grab an image.

        The auto mode is like an intervalometer to create timelapse animations.
        """
        self.frame_add()
        if self.config.verbose:
            print "intervalometer auto grab" 
            sys.stdout.flush()
        if self.intervalometer_on:
            self._intervalometer_delayed_id = reactor.callLater(self.intervalometer_rate_seconds, self._intervalometer_frame_add)

    def cleanup(self):
        """
        Called before quitting the application.
        """
        pass
        # glDeleteTextures(3, self.textures)

    def config_set(name, value):
        """
        Changes a configuration option.
        """
        self.config.set(name, value)

class Configuration(Serializable):
    """
    Configuration options.
    
    Default values are defined here. 
    Overriden as **argd arguments to ToonLoop(**argd)
    
    Python allows commas at the end of lists and tuples
    """
    def __init__(self, **argd): 
        self.toonloop_home = os.path.expanduser("~/Documents/toonloop")
        self.project_name = "new_project" # name of the folder
        self.max_num_shots = 10
        self.delete_jpeg = False
        self.image_width = 640
        self.image_height = 480
        self.display_width = 1024
        self.display_height = 768
        self.osc_send_port = 33333
        self.osc_send_host = 'localhost'
        self.osc_receive_port = 44444
        self.verbose = True
        self.min_framerate = 1
        self.max_framerate = 30
        self.min_framerate = 1
        self.max_framerate = 30
        self.onionskin_enabled = False
        self.web_server_port = 8000
        self.onionskin_opacity = 0.3
        self.playback_opacity = 0.3
        self.chromakey_enabled = True
        self.chromakey_on = True
        self.chromakey_r = 0.0
        self.chromakey_g = 1.0
        self.chromakey_b = 0.0
        self.chromakey_thresh = 0.5
        self.chromakey_on = True
        self.video_device = 0 
        self.intervalometer_on = False
        self.intervalometer_enabled = False
        self.intervalometer_rate_seconds = 30.0 # in seconds
        #self.min_framerate = 0
        #self.max_framerate = 30
        #self.project_file = 'project.txt'
        #self.keying_allowed = False
        #self.keying_color = (0.0, 1.0, 0.0)
        #self.keying_thresh = 0.3
        #self.keying_slope = 0.0
        #self.max_num_frames = 1000
        #self.osc_receive_hosts = ''
        # overrides some attributes whose defaults and names are below.
        self.__dict__.update(**argd) 
    
    def set(self, name, value):
        """
        Casts to its type and sets the value.
        """
        # try:
        kind = str(type(self.__dict__[name]))
        self.__dict__[name] = type(self.__dict__[name])(value)
        print 'set %s = %s (%s)' % (name, value, kind)
        return kind
        # except Exception, e:
        #    print e.message


