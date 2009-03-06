#!/usr/bin/env python
# ToonLoop for Python
#
# Copyright 2008 Tristan Matthews & Alexandre Quessy
# <le.businessman@gmail.com> & <alexandre@quessy.net>
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
# You should have received a copy of the GNU General Public License
# along with camLoop.py.  If not, see <http://www.gnu.org/licenses/>.
#
"""
ToonLoop is a realtime stop motion performance tool. 

The idea is to spread its use for teaching new medias to children and to 
give a professional tool for movie creators.

Camera module for pygame available from pygame's svn revision 1744 or greater
svn co svn://seul.org/svn/pygame/trunk
"""
import sys
from time import strftime
import os

import pygame
import pygame.camera
from pygame.locals import *
from pygame import time

from twisted.internet import reactor

__version__ = "1.0.1 alpha"

pygame.init()

class ToonLoopError(Exception):
    """
    Any error ToonLoop might encouter
    """
    pass

class ToonLoop(object):
    def __init__(self, **argd):
        """
        Startup poutine.
        """
        if os.uname()[0] == 'Darwin':
            self.isMac = True
        else:
            self.isMac = False

        self.img_width = 640
        self.height = 480 
        self.last_image = pygame.Surface((self.img_width, self.height))
        #super(ToonLoop, self).__init__(**argd) <-- ??
        self.running = True
        self.paused = False
        pygame.display.set_caption("ToonLoop")
        self.v4l2_device = "/dev/video0"
        
        self.__dict__.update(**argd) # overrides some attributes whose defaults and names are below
        self.width = self.img_width * 2
        self.size = (self.width, self.height)
        self.surface = pygame.display.set_mode(self.size)
        try:
            pygame.camera.init() 
        except Exception, e:
            print "error calling pygame.camera.init()", e.message
        try:
            print "cameras :", pygame.camera.list_cameras()
            if self.isMac:
                self.camera = pygame.camera.Camera(0, (self.img_width, self.height))
            else:
                self.camera = pygame.camera.Camera(self.v4l2_device, (self.img_width, self.height))
            self.camera.start()
        except SystemError, e:
            raise ToonLoopError("Invalid camera. %s" % (str(e.message)))
        self.clock = pygame.time.Clock()
        self.fps = 0 # for statistics
        self.image_list = []
        self.image_idx = 0
        self.timer = None # Twisted PygameTimer instance that owns it.

    def get_and_flip(self):
        """
        Image capture from the video camera and pygame pixels update.
        """
        if self.isMac:
            self.camera.get_image(self.last_image)
        else:
            self.last_image = self.camera.get_image()
        self.surface.blit(self.last_image, (0, 0))
        if len(self.image_list) > self.image_idx:
            self.surface.blit(self.image_list[self.image_idx], (self.img_width, 0))
            self.image_idx += 1
        else:
            self.image_idx = 0
        pygame.display.update()

    def grab_image(self):
        """
        Copies the last grabbed to the list of images.
        """
        if self.isMac:
            self.image_list.append(self.last_image.copy())
        else:
            self.image_list.append(self.last_image)

    def pause(self):
        """
        Toggles on/off the pause
        """
        self.paused = not self.paused

    def reset_loop(self):
        """
        Deletes all frames from the current animation
        """
        self.image_list = []
        self.reset_playback_window()

    def save_images(self):
        """
        Saves all images as jpeg
        """
        # TODO : in a thread or using reactor.callLater()
        datetime = strftime("%Y-%m-%d_%H:%M:%S")
        print "Saving images ", datetime, " " 
        reactor.callLater(0, self._save_next_image, datetime, 0)

    def _save_next_image(self, datetime, index):
        """
        Saves each image using twisted in order not to freeze the app.
        """
        if index < len(self.image_list):
            name = ("%s_%4d.jpg" % (datetime, index)).replace(' ', '0')
            sys.stdout.write("%d " % index)
            pygame.image.save(self.image_list[index], name)
            reactor.callLater(0, self._save_next_image, datetime, index + 1)
        else:
            print "" # done

    def pop_one_frame(self):
        """
        Deletes the last frame from the current list of images.
        """
        if self.image_list != []:
            self.image_list.pop()
            if self.image_list == []:
                self.reset_playback_window()

    def reset_playback_window(self):
        """
        Sets all pixels of the window as black.
        """
        blank_surface = pygame.Surface((self.img_width, self.height))
        playback_pos = (self.img_width, 0)
        self.surface.blit(blank_surface, playback_pos)

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
        print "<Esc> or q = quit program\n"

    def print_stats(self):
        """
        Print statistics
        """
        print "Frame idx: " + str(self.image_idx)
        print "Num images: " + str(len(self.image_list))
        print str(self.fps) + " fps\n"
    
    def increment_fps(self, dir=1):
        """
        Increase or decreases the FPS
        :param dir: by how much increment it.
        """
        if self.timer is not None:
            will_be = self.timer.desired_fps + dir
            if will_be > 0 and will_be <= 60:
                self.timer.desired_fps = will_be

    def draw(self):
        """
        Renders one frame.
        Called from the event loop. (twisted)
        """
        events = pygame.event.get()
        for e in events:
            if e.type == QUIT:
                self.running = False
            elif e.type == KEYDOWN: 
                if e.key == K_UP:
                    self.increment_fps(1)
                if e.key == K_DOWN:
                    self.increment_fps(-1)
                if e.key == K_SPACE:
                    self.grab_image()
                elif e.key == K_r:
                    self.reset_loop()
                elif e.key == K_p:
                    self.pause()
                elif e.key == K_i: 
                    self.print_stats()
                elif e.key == K_h:
                    self.print_help()
                elif e.key == K_s:
                    self.save_images()
                elif e.key == K_BACKSPACE:
                    self.pop_one_frame()
                elif e.key == K_ESCAPE or e.key == K_q:
                    self.running = False
        if not self.paused:
            self.get_and_flip()
            self.clock.tick()
            self.fps = self.clock.get_fps()
    
    def cleanup(self):
        """
        Called before quitting the application.
        """
        pass


class PygameTimer:
    """
    Integrates a pygame game and twisted.
    See http://twistedmatrix.com/pipermail/twisted-python/2002-October/001884.html
    """
    def __init__(self, game, verbose=False):
        self.clock = time.Clock()
        self.game = game
        self.game.timer = self 
        self.is_verbose = verbose
        self.desired_fps = 30.0
        # start
        self.update()

    def update(self):
        """
        Renders one frame sequenced using the Twisted events loop.
        """
        self.clock.tick()
        self.ms = self.clock.get_rawtime()
        
        framespeed = (1.0 / self.desired_fps) * 1000
        lastspeed = self.ms
        next = framespeed - lastspeed
        
        if self.is_verbose:
        #    print "framespeed", framespeed, "ms", self.ms, "next", next, "fps", self.clock.get_fps()
            print "FPS: %5f" % (self.clock.get_fps())
        
        # calls its draw method, which draw and refresh the whole screen.
        self.game.draw()
        
        if not self.game.running:
            self.game.cleanup()
            reactor.stop()
        else:
            when = next / 1000.0 * 2.0
            if when < 0:
                when = 0
            reactor.callLater(when, self.update)

if __name__ == "__main__":
    """
    Starts the application, reading the command-line arguments.
    """
    from optparse import OptionParser
    import sys

    parser = OptionParser(usage="%prog [version]", version=str(__version__))
    parser.add_option("-d", "--device", dest="device", default="/dev/video0", type="string", help="Specifies v4l2 device to grab image from.")
    parser.add_option("-v", "--verbose", dest="verbose", action="store_true", help="Sets the output to verbose.")
    
    (options, args) = parser.parse_args()
    
    # default options
    device = "/dev/video0" # TODO: use numbers not strings.
    is_verbose = False
    if options.device:
        device = options.device
    if options.verbose:
        is_verbose = True
        
    print "ToonLoop - Version " + str(__version__)
    print "Using v4l2 device " + device
    print "Press h for usage and instructions\n"
    try:
        toonloop = ToonLoop(v4l2_device=options.device, verbose=is_verbose)
    except ToonLoopError, e:
        print str(e.message)
        print "\nnow exiting"
        sys.exit(1)
    pygame_timer = PygameTimer(toonloop, is_verbose)
    
    reactor.run()

