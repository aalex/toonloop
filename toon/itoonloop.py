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
from toon.draw import draw_textured_square, draw_square
from toon import puredata
from toon import chromakey
try:
    from toon import web_server
    WEB_LOADED = True
except ImportError, e:
    print "For web support, please install the python-nevow package."
    print e.message
    WEB_LOADED = False

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

class ToonClip(Serializable):
    """
    ToonLoop clip.
    A clip is a serie of frames. 
    """
    def __init__(self, id, **argd):
        """
        :param id: int 
        """
        self.id = id
        self.playhead = 0
        self.playhead_iterate_every = 3 
        # Ratio that decides of the framerate
        # self.framerate = 12
        self.__dict__.update(argd)
        self.images = []

class ToonLoop(render.Game):
    """
    ToonLoop is a realtime stop motion tool.

    For 1 GB of RAM, one can expect to stock about 3000 images at 640x480, if having much swap memory.

    Private methods starts with _.
    The draw method is a special one inherited from Game.

    Onion skin effect and chroma key shader are mutually exclusives.
    """
    TEXTURE_MOST_RECENT = 0
    TEXTURE_PLAYBACK = 1
    TEXTURE_ONION = 2
    TEXTURE_BACKGROUND = 3

    def __init__(self, config):
        """
        Startup poutine.

        Reads config.
        Starts pygame, the camera, the clips list.
        Creates the window. Hides the mouse.
        Start services (OSC, web and FUDI)
        Enables the chromakey shader or the onion peal.
        """
        self.config = config
        # size of the rendering window
        self._display_size = (self.config.display_width, self.config.display_height)
        self.running = True
        self.pd = None
        self.paused = False
        self.image_size = (self.config.image_width, self.config.image_height)
        self.clock = pygame.time.Clock()
        self.fps = 0 # for statistics
        #self.images_list = [] # only editing one clip for now
        #self.clip.playhead = 0 
        self.clip_id = 0 # Currently selected clip
        self.clip = None # current ToonClip instance
        self.clips = [] # ToonClip instances
        self._init_clips()
        self.renderer = None # Renderer instance that owns it.
        # the icon
        icon = pygame.image.load(os.path.join(self.config.PACKAGE_PATH, 'data', "icon.png"))
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
        self.textures = [0, 0, 0, 0] # list of OpenGL texture objects id
        self._setup_camera()
        self._setup_window()
        self.background_image = None
        self._setup_background()
        self._clear_playback_view()
        self._clear_onion_peal()
        self._playhead_iterator = 0
        # intervalometer
        self._intervalometer_delayed_id = None
        self.intervalometer_on = self.config.intervalometer_on
        self._bgimage_glob_index = 0
        if config.intervalometer_on:
            self.intervalometer_toggle(True)
        # autosave
        self._autosave_delayed_id = None
        if config.autosave_on:
            self.autosave_toggle(True)
        # copy conf elements
        if self.config.verbose:
            pprint.pprint(self.config.__dict__)
        self._has_just_added_frame = False
        reactor.callLater(0, self._start_services)

    def _start_services(self):
        """
        Starts the TOonLoOp network services.

        Called once the Twisted reactor has been started. 

        Implemented services : 
         * web server with Media RSS and Restructured text
         * FUDI protocol with PureData
        """
        # OSC
        #self.osc = opensoundcontrol.ToonOsc(self)
        #index_file_path = os.path.join(os.curdir, 'toon', 'index.rst') 
        # WEB
        if WEB_LOADED and self.config.web_enabled:
            try:
                self.web = web_server.start(self, self.config.web_server_port,
                    static_files_path=self.config.toonloop_home)
                    #index_file_path=index_file_path)
            except:
                print "Error loading web UI :"
                print sys.exc_info()
        # FUDI
        if self.config.fudi_enabled:
            try:
                # TODO: subscription push mecanism
                app = self
                fudi_recv = self.config.fudi_receive_port
                fudi_send = self.config.fudi_send_port
                fudi_send_host = self.config.fudi_send_host

                self.pd = puredata.start(app=app, receive_port=fudi_recv, send_port=fudi_send, send_host=fudi_send_host)
            except:
                print "Error loading puredata:", sys.exc_info()
                raise

    def _setup_background(self):
        """
        Loads initial background image.
        """
        self.bgimage_load(self.config.bgimage)

    def bgimage_load(self, path):
        """
        Replace the background image by a new image file path.
        """
        if self.config.bgimage_enabled:
            self.config.bgimage = path
            if self.config.verbose:
                print 'setup background', path
        self.background_image = pygame.image.load(path)
        # Create an OpenGL texture
        texture_from_image(self.textures[self.TEXTURE_BACKGROUND], self.background_image)
    
    def bgimage_glob_next(self, increment=1):
        """
        Loads the next image from the list of JPG images in a directory.

        lowercase .jpg extension.
        """
        if self.config.bgimage_glob_enabled:
            dir = self.config.bgimage_glob
            ext = '.jpg'
            pattern = '%s/*%s' % (dir, ext)
            files = glob.glob(pattern)
            if self.config.verbose:
                print '----------------'
                print 'bgimage_glob_next pattern :', pattern
                print 'bgimage_glob_next len(files) :', len(files)
                # print 'bgimage_glob_next files :', sorted(files)
                # now = strftime("%Y-%m-%d_%Hh%Mm%S") # without an extension.
                # print 'bgimage_glob_next (debug) time is now :', now

            if len(files) > 0:
                old_val = self._bgimage_glob_index
                new_val = (self._bgimage_glob_index + increment) % len(files)
                print 'bgimage_glob_next old_val :', old_val
                print 'bgimage_glob_next new_val :', new_val
                if old_val == new_val:
                    if self.config.verbose:
                        print 'bgimage_glob_next same val. Not changing:', old_val
                else:
                    file_path = sorted(files)[new_val]
                    self._bgimage_glob_index = new_val 
                    print 'bgimage_glob_next self._bgimage_glob_index:', self._bgimage_glob_index
                    if self.config.verbose:
                        print 'bgimage_glob_next file_path:', file_path
                    self.bgimage_load(file_path)
                    # if self.config.verbose:
                    #     print 'bgimage_glob_next done'
                    #     print '----------------'
                    

    def print_stats(self):
        """
        Print statistics
        """
        print ">>>>>>> ToonLoop Statistics >>>>>>>>"
        try:
            print "Current playhead: " + str(self.clip.playhead)
            print "Num images: " + str(len(self.clip.images))
            print "FPS: %d" % (self.fps)
            print "Playhead frequency ratio: 30 / %d" % (self.clip.playhead_iterate_every)
            self.config.print_values()

            print 'pygame.display.Info(): ', pygame.display.Info()
            total_imgs = 0
            for clip_num in range(len(self.clips)):
                num_images = len(self.clips[clip_num].images)
                print 'Clip #%d has %d images.' % (clip_num, num_images)
                total_imgs += num_images
            print 'TOTAL: %d' % (total_imgs)
            print "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
        except AttributeError, e:
            print sys.exc_info()

    def print_help(self):
        """
        Prints help for live keyboard controls.
        """
        print "ToonLoop keyboard controls : "
        print "<Space bar> = add image to loop "
        print "<Backspace> = remove image from loop "
        print "r = reset loop"
        print "p = pause"
        print "o = toggle onion skin on/off"
        print "i = prints informations"
        print "h = prints this help message"
        print "s = saves current clip as jpeg and movie"
        print "a = enable the intervalometer auto grab"
        print "k = increase the intervalometer interval"
        print "j = decrease the intervalometer interval"
        print "[1, 9]  = select a clip from the current project"
        print "<Esc> = quit program\n"

    def _init_clips(self):
        """
        Creates a new project.
        """
        for i in range(self.config.max_num_clips):
            self.clips.append(ToonClip(i))
        try:
            self.clip = self.clips[self.clip_id]
        except IndexError:
            self.clip_id = 0
            self.clip = self.clips[self.clip_id]

    def clip_select(self, index=0):
        """
        Selects an other clip
        """
        self.clip_id = index
        self.clip = self.clips[index]
        if self.config.verbose:
            print "Clip #%s"  % (self.clip_id)
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

    def frame_add(self):
        """
        Copies the last grabbed frame to the list of images.
        """
        try:
            if self.is_mac:
                self.clip.images.append(self.most_recent_image.copy())
            else:
                self.clip.images.append(self.most_recent_image)
            # Creates an OpenGL texture
            texture_from_image(self.textures[self.TEXTURE_ONION], self.most_recent_image)
            self._has_just_added_frame = True
        except MemoryError, e:
            print "CRITICAL ERROR : No more RAM Memory !!!", e.message
        if self.config.verbose:
            print 'num frames', len(self.clip.images)
    
    def draw(self):
        """
        Renders one frame.
        Called from the event loop. (twisted)
        """
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        if not self.paused:
            self._playhead_iterator = (self._playhead_iterator + 1) % self.clip.playhead_iterate_every
            if self._playhead_iterator ==  0:
                self._playhead_iterate()
                # 30/3 = 10 FPS
        self._camera_grab_frame() # grab a frame
        CHROMA_ON = self.config.chromakey_enabled and self.config.chromakey_on
        # TODO: replace by effect number

        # now, let's draw something
        self._draw_background()
        # --------- edit view
        if CHROMA_ON: 
            chromakey.program_enable()
            chromakey.set_program_uniforms()
        else:
            chromakey.program_disable()
            
        self._draw_edit_view()
        if CHROMA_ON:
            chromakey.program_disable()
        # ---------- onion skin
        if CHROMA_ON:
            chromakey.program_enable()
            chromakey.set_program_uniforms()
        if self.config.onionskin_enabled and self.config.onionskin_on:
            self._draw_onion_skin()
        if CHROMA_ON:
            chromakey.program_disable()
        if self._has_just_added_frame:
            self._has_just_added_frame = False
            if self.config.fx_white_flash:
                self._draw_white_flash()

        # ---------- playback view
        if CHROMA_ON:
            chromakey.program_enable()
            chromakey.set_program_uniforms()
        self._draw_playback_view()
        if CHROMA_ON:
            chromakey.program_disable()
        
        self.clock.tick()
        self.fps = self.clock.get_fps()
        pygame.display.flip()
        # old : pygame.display.update()

    def _draw_white_flash(self):
        # TODO: use time.time() to create tween.
        if self.config.verbose:
            print 'white flash'
        # left view
        a = self.config.fx_white_flash_alpha
        glColor4f(1.0, 1.0, 1.0, a)
        glPushMatrix()
        glTranslatef(-2.0, 0.0, 0.0)
        glScalef(2.0, 1.5, 1.0)
        draw_square()
        glPopMatrix()

    def _draw_background(self):
        """
        Renders the background 
        """
        # r = self.config.bgcolor_r
        # g = self.config.bgcolor_g
        # b = self.config.bgcolor_b

        # glColor(r, g, b, 1.0)
        # glPushMatrix()
        # glScalef(4.0, 3, 1.0)
        # draw_square()
        # glPopMatrix()
        # glColor(1.0, 1.0, 1.0, 1.0)

        if self.config.bgimage_enabled:
            # left view
            glColor4f(1.0, 1.0, 1.0, 1.0)
            glPushMatrix()
            glTranslatef(-2.0, 0.0, 0.0)
            glScalef(2.0, 1.5, 1.0)
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.textures[self.TEXTURE_BACKGROUND])
            draw_textured_square(self.config.image_width, self.config.image_height)
            glPopMatrix()
            # right view
            glPushMatrix()
            glTranslatef(2.0, 0.0, 0.0)
            glScalef(2.0, 1.5, 1.0)
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.textures[self.TEXTURE_BACKGROUND])
            draw_textured_square(self.config.image_width, self.config.image_height)
            glPopMatrix()

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
        if self.clip.playhead < len(self.clip.images):
            image = self.clip.images[self.clip.playhead]
            texture_from_image(self.textures[self.TEXTURE_PLAYBACK], image)
            self.clip.playhead += 1
            # old: self.display.blit(self.clip.images[self.clip.playhead], (self.config.image_width, 0))
        else:
            self.clip.playhead = 0

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

    def clip_reset(self):
        """
        Deletes all frames from the current animation
        """
        self.clip.images = []
        self._clear_playback_view()

    def clip_save(self):
        """
        Saves all images as jpeg and encodes them to a MJPEG movie.
        
        See _write_next_image
        """
        # TODO : in a thread
        if len(self.clip.images) > 1:
            file_name = strftime("%Y-%m-%d_%Hh%Mm%S") # without an extension.
            path = os.path.join(self.config.toonloop_home, self.config.project_name)
            if self.config.verbose:
                print "Saving images ", path, file_name
            try:
                if not os.path.exists(path):
                    os.makedirs(path)
                data_subdir = os.path.join(path, 'data')
                if not os.path.exists(data_subdir):
                    os.makedirs(data_subdir)
            except OSError, e:
                print 'Error creating directories', path, e.message
            reactor.callLater(0, self._write_01_next_image, path, file_name, 0)

    def _write_01_next_image(self, path, file_name, index):
        """
        Saves each image using twisted in order not to freeze the app.
        Uses the JPG extension.
        """
        if index < len(self.clip.images):
            name = ("%s/%s_%5d.jpg" % (path, file_name, index)).replace(' ', '0')
            if self.config.verbose:
                print "writing %s" % (name)
            pygame.image.save(self.clip.images[index], name) # filename extension makes it a JPEG
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
            #self.clip.increment_every # self.clip.framerate
            #fps = self.renderer.desired_fps 

            deferred = mencoder.jpeg_to_movie(file_name, path, fps, self.config.verbose, self.config.image_width, self.config.image_height)
            deferred.addCallback(self._write_03_movie_done, file_name, path, index)
            # to do : serialize clips with file names
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

        files = glob.glob("%s/%s_*.jpg" % (path, file_name))
        for f in files:
            if self.config.delete_jpeg:
                try:
                    os.remove(f)
                    if self.config.verbose:
                        print 'removed', f
                except OSError, e:
                    print "%s Error removing file %s" % (e.message, f)
            else:
                try:
                    dest = os.path.join(path, 'data', os.path.basename(f))
                    shutil.move(f, dest)
                    if self.config.verbose:
                        print 'moved', f, dest
                except IOError, e:
                    print "%s Error moving file %s to %s" % (e.message, f, dest)
                
        # rename final movie file
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
        if self.clip.images != []:
            self.clip.images.pop()
            # would it be better to also delete it ? calling del
            if self.clip.images == []:
                self._clear_playback_view()
            else:
                pass 
                #texture_from_image(self.textures[self.TEXTURE_ONION], self.most_recent_image)
    
    def chromakey_toggle(self, val=None):
        """
        Mutually exclusive with onionskin_toggle
        """
        if val is not None:
            self.config.chromakey_on = val is True # set
        else:
            self.config.chromakey_on = not self.config.chromakey_on # toggle
        print 'config.chromakey_on =', self.config.chromakey_on
        # check chroma key and disables it if so
        if self.config.chromakey_on:
            if self.config.onionskin_on:
                self.config.onionskin_on = False
                print 'config.onionskin_on =', self.config.onionskin_on
    
    def effect_select(self, index=0):
        """
        Selects an effect.

        Current choices :
        * 0 : None
        * 1 : chromakey
        * 2 : onionskin
        """

        if index == 1:
            if self.config.verbose:
                print 'EFFECT: chromakey'
            self.config.chromakey_on = True
            self.config.onionskin_on = False
        elif index == 2:
            if self.config.verbose:
                print 'EFFECT: onionskin'
            self.config.chromakey_on = False
            self.config.onionskin_on = True
        else:
            if self.config.verbose:
                print 'EFFECT: None'
            self.config.chromakey_on = False
            self.config.onionskin_on = False

    def onionskin_toggle(self, val=None):
        """
        Toggles on/off the onion skin. 
        (see most recent frame grabbed in transparency)
        Mutually exclusive with chromakey_toggle
        """
        if val is not None:
            self.config.onionskin_on = val is True # set to val
        else:
            self.config.onionskin_on = not self.config.onionskin_on # toggle
        print 'config.onionskin_on =', self.config.onionskin_on
        # check onionskin and disables it if so
        if self.config.onionskin_on:
            if self.config.chromakey_on:
                self.config.chromakey_on = False
                print 'config.chromakey_on =', self.config.chromakey_on
            
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
        will_be = self.clip.playhead_iterate_every - dir
        if will_be > self.config.framerate_min and will_be <= self.config.framerate_max:
            self.clip.playhead_iterate_every = will_be
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
                elif e.key == K_c: # C : chromakey toggle
                    # self.config.chromakey_on
                    self.chromakey_toggle()
                elif e.key == K_j: # J
                    self.intervalometer_rate_increase(-1)
                elif e.key == K_f: # F Fullscreen
                    pygame.display.toggle_fullscreen()
                elif e.key == K_i: # I Info
                    try:
                        self.print_stats()
                    except Exception, e:
                        print e.message
                elif e.key == K_p: # P Pause
                    self.pause()
                elif e.key == K_r: # R Reset
                    self.clip_reset()
                elif e.key == K_h: # H Help
                    self.print_help()
                elif e.key == K_s: # S Save
                    self.clip_save()
                elif e.key == K_o: # O Onion
                    self.onionskin_toggle()
                elif e.key == K_a: # A Auto
                    print "toggle intervalometer"
                    self.intervalometer_toggle()
                elif e.key == K_0: # [0, 9] Clip selection
                    self.clip_select(0)
                elif e.key == K_1:
                    self.clip_select(1)
                elif e.key == K_2:
                    self.clip_select(2)
                elif e.key == K_3:
                    self.clip_select(3)
                elif e.key == K_4:
                    self.clip_select(4)
                elif e.key == K_5:
                    self.clip_select(5)
                elif e.key == K_6:
                    self.clip_select(6)
                elif e.key == K_7:
                    self.clip_select(7)
                elif e.key == K_8:
                    self.clip_select(8)
                elif e.key == K_9:
                    self.clip_select(9)
                elif e.key == K_UP: # UP Speed Increase
                    self.framerate_increase(1)
                elif e.key == K_DOWN: # DOWN Speed Decrease
                    self.framerate_increase(-1)
                elif e.key == K_SPACE: # SPACE Add frame
                    self.frame_add()
                elif e.key == K_BACKSPACE: # BACKSPACE Remove frame
                    self.frame_remove()
                elif e.key == K_ESCAPE: #  or e.key == K_q: # ESCAPE or Q
                    self.quit()
    def quit(self):
        """Quits the application in a short while.
        """
        reactor.callLater(0.1, self._quit)
    def _quit(self):
        self.running = False

    def autosave_toggle(self, val=None):
        """
        Toggles on/off the autosave 
        """
        if self.config.autosave_enabled:
            if val is not None:
                self.config.autosave_on = val
            else:
                self.config.autosave_on = not self.config.autosave_on
            if self.config.autosave_on:
                self._autosave_delayed_id = reactor.callLater(0, self._autosave)
                if self.config.verbose:
                    print "autosave ON"
            else:
                if self._autosave_delayed_id is not None:
                    if self._autosave_delayed_id.active():
                        self._autosave_delayed_id.cancel()
                        if self.config.verbose:
                            print "autosave OFF"
                
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
            will_be = self.config.intervalometer_rate_seconds + dir
            if will_be > 0 and will_be <= 60:
                self.config.intervalometer_rate_seconds = will_be
                print "auto rate:", will_be

    def _intervalometer_frame_add(self):
        """
        Called when it is time to automatically grab an image.

        The auto mode is like an intervalometer to create timelapse animations.
        """
        self.frame_add()
        if self.config.verbose:
            print "intervalometer auto grab", len(self.clip.images) 
            sys.stdout.flush()
        if self.intervalometer_on:
            self._intervalometer_delayed_id = reactor.callLater(self.config.intervalometer_rate_seconds, self._intervalometer_frame_add)

    def _autosave(self):
        """
        Called when it is time to automatically save a movie
        """
        # self.frame_add()
        if self.config.verbose:
            print "autosave"
        self.clip_save()
        if self.config.autosave_on:
            self._autosave_delayed_id = reactor.callLater(self.config.autosave_rate_seconds, self._autosave)

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
        # constants
        self.PACKAGE_PATH = os.path.dirname(toon.__file__)
        
        # basics
        self.verbose = True
        self.video_device = 0 
        
        # project
        self.toonloop_home = os.path.expanduser("~/Documents/toonloop")
        self.project_name = "new_project" # name of the folder
        #self.project_file = 'project.txt'
        self.max_num_clips = 10
        self.delete_jpeg = False
        
        # framerate
        self.framerate_min = 1
        self.framerate_max = 30
        # TODO: framerate value

        # image size
        self.image_width = 320 # 640
        self.image_height = 240 # 480
        #self.playback_opacity = 0.3
        #self.max_num_frames = 1000
        
        # window
        self.display_width = 1024
        self.display_height = 768
        
        # web services
        self.web_server_port = 8000
        self.web_enabled = False
        
        # fudi puredata interface
        self.fudi_enabled = False
        self.fudi_receive_port = 15555
        self.fudi_send_port = 17777
        self.fudi_send_host = 'localhost'
        
        # osc is not implemented right now.
        #self.osc_send_port = 33333
        #self.osc_send_host = 'localhost'
        #self.osc_receive_port = 44444
        #self.osc_receive_hosts = ''
        
        # onionskin
        self.onionskin_enabled = True
        self.onionskin_on = False
        self.onionskin_opacity = 0.3
        
        # background
        self.bgimage_enabled = True
        self.bgimage = os.path.join(self.PACKAGE_PATH, 'data/bgimage_05.jpg')
        self.bgcolor_b = 0.2 #TODO: not used so much.
        self.bgcolor_g = 0.8
        self.bgcolor_r = 1.0
        self.bgimage_glob_enabled = False # list of glob JPG files that can be browsed using +/- iteration
        self.bgimage_glob = os.path.join(self.toonloop_home, self.project_name, 'data') # defaults to the images from the current project !
        
        # white flash
        self.fx_white_flash = True
        self.fx_white_flash_alpha = 1.0
        # todo:duration
        
        # chromakey
        self.chromakey_enabled = True
        self.chromakey_on = False
        self.chromakey_r = 0.2
        self.chromakey_g = 0.9
        self.chromakey_b = 0.0
        self.chromakey_thresh = 0.5
        self.chromakey_slope = 0.2
        
        # intervalometer
        self.intervalometer_on = False 
        self.intervalometer_enabled = False
        self.intervalometer_rate_seconds = 30.0 # in seconds
        # TODO: clean intervalometer on/enabled stuff
        
        # autosave
        self.autosave_on = False 
        self.autosave_enabled = True
        self.autosave_rate_seconds = 600.0 # in seconds
        
        # overrides some attributes whose defaults and names are below.
        self.__dict__.update(**argd) 

    def print_values(self):
        for k in sorted(self.__dict__):
            v = self.__dict__[k]
            print "    -o %s %s" % (k, v)

    def set(self, name, value):
        """
        Casts to its type and sets the value.

        Intended to be used even from ASCII string values. (FUDI, etc.)

        A bool value can be set using 'True' or 1 and 'False' or 0
        """
        # try:
        kind = type(self.__dict__[name])
        if kind is bool:
            if value == 'True':
                casted_value = True
            elif value == 'False':
                casted_value = False
            else:
                casted_value = bool(int(value))
        else:
            casted_value = kind(value)
        self.__dict__[name] = casted_value
        print 'config.%s = %s(%s)' % (name, kind.__name__, self.__dict__[name])
        return kind
        # except Exception, e:
        #    print e.message


