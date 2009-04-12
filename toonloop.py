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
"""
# TODO:
# - Press numbers from 0 to 9 to switch to an other sequence.
# - Press 'p' to open the Quicktime video camera settings dialog. (if available)
# - Press LEFT or RIGHT to move the insertion point
# - OSC messages to set intervalometer rate (timelapse) and enable it.
# - onion peal
# - text for frame number on both sides
# - OSC callbacks and sends

import sys
from time import strftime
import os

from toon import opensoundcontrol
from toon import mencoder
from rats import render
from rats.serialize import Serializable
from rats.observer import Subject

import pygame
import pygame.camera
from pygame.locals import *
from pygame import time
from OpenGL.GL import *

from twisted.internet import reactor

__version__ = "1.0 beta"

def texture_from_image(texture, image):
    """
    Copies the pixels from a pygame surface to an OpenGL texture object.
    """
    textureData = pygame.image.tostring(image, "RGBX", 1)
    glBindTexture(GL_TEXTURE_2D, texture)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.get_width(), image.get_height(), 0, \
              GL_RGBA, GL_UNSIGNED_BYTE, textureData)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)

def draw_textured_square():
    """
    Draws a texture square
    """
    glBegin(GL_QUADS)
    glTexCoord2f(0.0, 0.0)
    glVertex2f(-1.0, -1.0) # Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0, 0.0)
    glVertex2f(1.0, -1.0) # Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0, 1.0)
    glVertex2f(1.0, 1.0) # Top Right Of The Texture and Quad
    glTexCoord2f(0.0, 1.0)
    glVertex2f(-1.0, 1.0) # Top Left Of The Texture and Quad
    glEnd()

class ToonSequence(Serializable):
    """
    Not used yet !!!!!!
    
    ToonLoop sequence. 

    A sequence is made of many shots.
    
    An act include one or more sequences; sequences comprise one or more scenes; 
    and scenes may be thought of as being built out of shots (if one is thinking visually) 
    or beats (if one is thinking in narrative terms).
    """
    # TODO: use it !
    def __init__(self, **argd):
        self.shots_ids = []
        # end of overridable attributes
        self.__dict__.update(argd)

class ToonShot(Serializable):
    """
    Not used yet !!!!!!

    ToonLoop shot.

    A shot is a serie of frames. 
    """
    # TODO: use it !
    def __init__(self, **argd):
        self.writehead_position = 0
        self.playhead_position = 0
        self.images_files = []
        self.framerate = 12
        self.intervalometer_enabled = False
        self.intervalometer_rate_seconds = 0.1
        # end of overridable attributes
        self.__dict__.update(argd)
        self._intervalometer_delayed_id = None
        self.images_list = []

class ToonLoopError(Exception):
    """
    Any error ToonLoop might encouter
    """
    pass

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
        print "Usage: "
        print "<Space bar> = add image to loop "
        print "<Backspace> = remove image from loop "
        print "r = reset loop"
        print "p = pause"
        print "i = print current loop frame number, number of frames in loop and global framerate"
        print "h = print this help message"
        print "s = saves all images as jpeg"
        print "a = enable the intervalometer auto grab"
        print "k = increase the intervalometer interval"
        print "j = decrease the intervalometer interval"
        print "<Esc> or q = quit program\n"

    def print_stats(self):
        """
        Print statistics
        """
        try:
            print "Current playhead: " + str(self.app.current_playhead)
            print "Num images: " + str(len(self.app.images_list))
            print "FPS: %d" % (self.app.fps)
            print "Playhead frequency ratio: 30 / %d" % (self.app.playhead_iterate_every)
        except AttributeError, e:
            print sys.exc_info()

class Configuration(Serializable):
    """
    Not used yet
    """
    # TODO: use it !
    def __init__(self, **argd): 
        self.max_num_frames = 1000
        self.max_num_shots = 10
        self.image_width = 640
        self.image_height = 480
        self.display_width = 640 * 2
        self.display_height = 480 * 2
        self.min_framerate = 0
        self.max_framerate = 30
        self.osc_send_port = 33333
        self.osc_send_host = 'localhost'
        self.osc_receive_port = 44444
        self.osc_receive_hosts = ''
        self.onionpeal_allowed = True
        self.onionpeal_opacity = 0.3
        self.keying_allowed = False
        self.keying_color = (0.0, 1.0, 0.0)
        self.toonloop_home = os.getcwd()
        self.toonloop_file_extension = 'txt'
        self.keying_thresh = 0.3
        self.keying_slope = 0.0
        # TODO: think about keying
        # end of overridable attributes
        self.__dict__.update(**argd) # overrides some attributes whose defaults and names are below

class ToonLoop(render.Game):
    """
    ToonLoop is a realtime stop motion tool.
    """
    TEXTURE_MOST_RECENT = 0
    TEXTURE_PLAYBACK = 1
    TEXTURE_ONION = 2

    def __init__(self, **argd):
        """
        Startup poutine.
        """
        self.config = Configuration(**argd)
        self.api = Api(self)
        self.video_device = 0 
        self.running = True
        self.paused = False
        self.__dict__.update(**argd) # overrides some attributes whose defaults and names are below
        self._display_size = (self.config.image_width * 2, self.config.image_height * 2) # size of the rendering window
        self.image_size = (self.config.image_width, self.config.image_height)
        self.clock = pygame.time.Clock()
        self.fps = 0 # for statistics
        self.images_list = [] # only editing one shot for now
        self.current_playhead = 0 
        self.renderer = None # Renderer instance that owns it.
        self.intervalometer_on = False
        self.intervalometer_rate_seconds = 1.0 # in seconds
        self._intervalometer_delayed_id = None
        # specific to pygame surface window
        # the screen's surface
        pygame.display.set_caption("ToonLoop")
        self.display = pygame.display.set_mode(self._display_size, OPENGL | DOUBLEBUF | HWSURFACE)
        # the images
        self.most_recent_image = pygame.surface.Surface(self.image_size) # , 0, self.display)
        
        self.osc = None
        self.camera = None
        self.is_mac = False
        self.textures = [0, 0, 0] # list of OpenGL texture objects id
        self._setup_camera()
        self._setup_window()
        self._clear_playback_view()
        self._clear_onion_peal()
        self._playhead_iterator = 0
        self.playhead_iterate_every = 3 # 30/3 = 10 FPS
        reactor.callLater(0, self._setup_services)

    def _setup_window(self):
        """
        OpenGL setup.
        """
        # create OpenGL texture objects 
        # window is 1280 x 960
        self._resize_window(self._display_size)
        glEnable(GL_TEXTURE_2D)
        glEnable(GL_BLEND)
        glShadeModel(GL_SMOOTH)
        glClearColor(0.0, 0.0, 0.0, 0.0) # black background
        glColor4f(1.0, 1.0, 1.0, 1.0)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        for i in range(len(self.textures)):
            self.textures[i] = glGenTextures(1)
        # print "texture names : ", self.textures

    def _resize_window(self, (width, height)):
        """
        Called when we resize the window.
        (fullscreen on/off)

        The OpenGL coordinates go from -4 to 4 horizontally
        and -3 to 3 vertically.
        (ratio is 4:3)
        """
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
        except Exception, e:
            print "error calling pygame.camera.init()", e.message
            print sys.exc_info()
            raise ToonLoopError("Error initializing the video camera. %s" % (e.message))
        try:
            print "cameras :", pygame.camera.list_cameras()
            if self.is_mac:
                print "Using camera %s" % (self.video_device)
                self.camera = pygame.camera.Camera(self.video_device, size)
            else:
                print "Using camera /dev/video%d" % (self.video_device)
                self.camera = pygame.camera.Camera("/dev/video%d" % (self.video_device), size)
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
        self.osc = opensoundcontrol.ToonOsc(self)

    def frame_add(self):
        """
        Copies the last grabbed frame to the list of images.
        """
        if self.is_mac:
            self.images_list.append(self.most_recent_image.copy())
        else:
            self.images_list.append(self.most_recent_image)
        # Create an OpenGL texture
        texture_from_image(self.textures[self.TEXTURE_ONION], self.most_recent_image)
    
    def draw(self):
        """
        Renders one frame.
        Called from the event loop. (twisted)
        """
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        if not self.paused:
            self._playhead_iterator = (self._playhead_iterator + 1) % self.playhead_iterate_every
            if self._playhead_iterator ==  0:
                self._playhead_iterate()
                # 30/3 = 10 FPS
        self._camera_grab_frame() # grab a frame
        self._draw_edit_view() # render edit view
        self._draw_playback_view() # render playback view
        self.clock.tick()
        self.fps = self.clock.get_fps()
        #pygame.display.update() # old
        pygame.display.flip()

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
        #self.display.blit(self.most_recent_image, (0, 0))
        glPushMatrix()
        glTranslatef(-2.0, 0.0, 0.0)
        glScalef(2.0, 1.5, 1.0)
        # most recent grabbed :
        glBindTexture(GL_TEXTURE_2D, self.textures[self.TEXTURE_MOST_RECENT])
        draw_textured_square()
        # Onion peal :
        glColor4f(1.0, 1.0, 1.0, 0.3)
        glBindTexture(GL_TEXTURE_2D, self.textures[self.TEXTURE_ONION])
        draw_textured_square()
        glColor4f(1.0, 1.0, 1.0, 1.0)
        glPopMatrix()

    def _draw_playback_view(self):
        """
        Renders the playback view. 
        """
        glPushMatrix()
        glTranslatef(2.0, 0.0, 0.0)
        glScalef(2.0, 1.5, 1.0)
        glBindTexture(GL_TEXTURE_2D, self.textures[self.TEXTURE_PLAYBACK])
        draw_textured_square()
        glPopMatrix()
    
    def _playhead_iterate(self):
        """ 
        Increments the playhead position of one frame
        """
        if self.current_playhead < len(self.images_list):
            #self.display.blit(self.images_list[self.current_playhead], (self.config.image_width, 0))
            image = self.images_list[self.current_playhead]
            texture_from_image(self.textures[self.TEXTURE_PLAYBACK], image)
            self.current_playhead += 1
        else:
            self.current_playhead = 0

    def _clear_playback_view(self):
        """
        Sets all pixels in the playback view as black.
        """
        blank_surface = pygame.Surface((self.config.image_width, self.config.image_height))
        texture_from_image(self.textures[self.TEXTURE_PLAYBACK], blank_surface)
        playback_pos = (self.config.image_width, 0)
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
        self.images_list = []
        self._clear_playback_view()

    def shot_save(self):
        """
        Saves all images as jpeg and encodes them to a MJPEG movie.
        
        See _write_next_image
        """
        # TODO : in a thread
        datetime = strftime("%Y-%m-%d_%Hh%Mm%S")
        print "Saving images ", datetime, " " 
        reactor.callLater(0, self._write_next_image, datetime, 0)

    def _write_next_image(self, datetime, index):
        """
        Saves each image using twisted in order not to freeze the app.
        """
        if index < len(self.images_list):
            name = ("%s_%4d.jpg" % (datetime, index)).replace(' ', '0')
            sys.stdout.write("%d " % index)
            pygame.image.save(self.images_list[index], name)
            reactor.callLater(0, self._write_next_image, datetime, index + 1)
        else:
            print "\nConverting to mjpeg" # done
            filename_pattern = "%s_" % (datetime)
            path = os.getcwd() # TODO: use config
            fps = self.renderer.desired_fps 
            deferred = mencoder.jpeg_to_movie(filename_pattern, path, fps)

    def frame_remove(self):
        """
        Deletes the last frame from the current list of images.
        """
        if self.images_list != []:
            self.images_list.pop()
            # would it be better to also delete it ? calling del
            if self.images_list == []:
                self._clear_playback_view()
    
    def framerate_increase(self, dir=1):
        """
        Increase or decreases the FPS
        :param dir: by how much increment it.
        """
        #TODO : do not alter the FPS of the renderer, but rather only the playback rate.
        # if self.renderer is not None:
        #     # accesses the Renderer instance that owns this.
        #     will_be = self.renderer.desired_fps + dir
        #     if will_be > 0 and will_be <= 60:
        #         self.renderer.desired_fps = will_be
        #         print "FPS:", will_be
        will_be = self.playhead_iterate_every - dir
        if will_be > 0 and will_be <= 30:
            self.playhead_iterate_every = will_be
            print "Playhead frequency ratio: 30 / %d" % (will_be)
    
    def process_events(self, events):
        """
        Processes pygame events.
        :param events: got them using pygame.event.get()
        """
        for e in events:
            if e.type == QUIT:
                self.running = False
            elif e.type == KEYDOWN: 
                if e.key == K_k:
                    self.intervalometer_rate_increase(1)
                elif e.key == K_j:
                    self.intervalometer_rate_increase(-1)
                elif e.key == K_UP:
                    self.framerate_increase(1)
                elif e.key == K_DOWN:
                    self.framerate_increase(-1)
                elif e.key == K_SPACE:
                    self.frame_add()
                elif e.key == K_f:
                    pygame.display.toggle_fullscreen()
                elif e.key == K_r:
                    self.shot_reset()
                elif e.key == K_p:
                    self.pause()
                elif e.key == K_i: 
                    self.api.print_stats()
                elif e.key == K_h:
                    self.api.print_help()
                elif e.key == K_s:
                    self.shot_save()
                elif e.key == K_a:
                    self.intervalometer_toggle()
                elif e.key == K_BACKSPACE:
                    self.frame_remove()
                elif e.key == K_ESCAPE or e.key == K_q:
                    self.running = False
            elif e.type == pygame.VIDEORESIZE:
                print "VIDEORESIZE", e
    
    def intervalometer_toggle(self, val=None):
        """
        Toggles on/off the auto mode
        """
        if val is not None:
            self.intervalometer_on = val
        else:
            self.intervalometer_on = not self.intervalometer_on
        if self.intervalometer_on:
            self._intervalometer_delayed_id = reactor.callLater(0, self._intervalometer_frame_add)
            print "enabled intervalometer"
        elif self._intervalometer_delayed_id.active():
            self._intervalometer_delayed_id.cancel()
            print "disabled intervalometer"
    
    def intervalometer_rate_increase(self, dir=1):
        """
        Increase or decreases the intervalometer rate. (in seconds)
        :param dir: by how much increment it.
        """
        will_be = self.intervalometer_rate_seconds + dir
        if will_be > 0 and will_be <= 60:
            self.intervalometer_rate_seconds = will_be
            print "auto rate:", will_be

    def _intervalometer_frame_add(self):
        """
        Called when it is time to utomatically grab an image.

        The auto mode is like an intervalometer to create timelapse animations.
        """
        self.frame_add()
        print "grab", # without endline character
        if self.intervalometer_on:
            self._intervalometer_delayed_id = reactor.callLater(self.intervalometer_rate_seconds, self._intervalometer_frame_add)

    def cleanup(self):
        """
        Called before quitting the application.
        """
        pass
        # glDeleteTextures(3, self.textures)

if __name__ == "__main__":
    """
    Starts the application, reading the command-line arguments.
    """
    from optparse import OptionParser

    parser = OptionParser(usage="%prog [version]", version=str(__version__))
    parser.add_option("-d", "--device", dest="device", default=0, type="int", \
        help="Specifies v4l2 device to grab image from.")
    parser.add_option("-v", "--verbose", dest="verbose", action="store_true", \
        help="Sets the output to verbose.")
    parser.add_option("-f", "--fps", type="int", default=30, \
        help="Sets the rendering frame rate.")
    parser.add_option("-t", "--intervalometer", type="int", default=3, \
        help="Sets intervalometer interval in seconds.")
    parser.add_option("-i", "--enable-intervalometer", \
        dest="enable_intervalometer", action="store_true", \
        help="Enables the intervalometer at startup.")
    (options, args) = parser.parse_args()
    
    print "ToonLoop - Version " + str(__version__)
    print "Copyright 2008 Alexandre Quessy & Tristan Matthews"
    print "Released under the GNU General Public License"
    print "Using video device %d" % options.device
    print "Press h for usage and instructions\n"
    #print "options:", options
    
    pygame.init()
    try:
        toonloop = ToonLoop(video_device=options.device, \
            intervalometer_rate_seconds=options.intervalometer, \
            intervalometer_on=options.enable_intervalometer == True, \
            verbose=options.verbose == True)
    except ToonLoopError, e:
        print str(e.message)
        print "\nnow exiting"
        sys.exit(1)
    pygame_timer = render.Renderer(toonloop, options.verbose)
    pygame_timer.desired_fps = options.fps
    try:
        reactor.run()
    except KeyboardInterrupt:
        pass
    print "\nExiting toonloop"
    sys.exit(0)

