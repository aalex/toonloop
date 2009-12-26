#!/usr/bin/env python
#
# Toonloop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# <alexandre@quessy.net> & <le.businessman@gmail.com>
# http://www.toonloop.com
#
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# Toonloop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Toonloop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
#
"""
Renderer for Pygame using Twisted
"""

import pygame
from pygame import time
from twisted.internet import reactor
from twisted.internet import task

class Game(object):
    """
    A pygame game
    """
    def __init__(self):
        self.renderer = None
        self.running = True

    def draw(self):
        """
        Called on every frame
        """
        raise NotImplementedError("You must implement this in child classes.")

    def process_events(self, pygame_events):
        """
        Called when pygame event occur
        """
        pass

    def cleanup(self):
        """
        Called just before exiting the application
        """
        pass

class Renderer(object):
    """
    Integrates a pygame game and twisted.
    See http://twistedmatrix.com/pipermail/twisted-python/2002-October/001884.html
    """
    def __init__(self, game, verbose=False):
        self.clock = time.Clock()
        self.game = game
        self.game.renderer = self 
        self.is_verbose = verbose
        self.desired_fps = 12.0
        # start
        self.check_for_events() # starts a metronome
        self._looping_events_check = None
        self._setup_events_looping_call()
        self.update() # starts a metronome

    def _on_events_error(self, reason):
        """
        ErrBack for the self._looping_events_check LoopingCall.
        """
        reason.printTraceback()
        #print(reason.getErrorMessage())
        self._setup_events_looping_call()

    def _setup_events_looping_call(self):
        """
        Sets up the self._looping_events_check LoopingCall.
        """
        self._looping_events_check = task.LoopingCall(self.check_for_events)
        deferred = self._looping_events_check.start(0.01, True) # now=True
        # FIXME: events are check at 100 fps ! Make configurable.
        deferred.addErrback(self._on_events_error)
    
    def check_for_events(self):
        """
        Check for pygame events and warn the game.
        :param events: got them using pygame.event.get()
        """
        events = pygame.event.get()
        self.game.process_events(events)
        #reactor.callLater(0, self.check_for_events)

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
            print "FPS: %5f" % (self.clock.get_fps())
        # calls its draw method, which draw and refresh the whole screen.
        self.game.draw()
# FIXME: if an exception is raised here, no more drawing occurs !!
        if not self.game.running:
            self._stop()
        else:
            when = next / 1000.0 * 2.0
            if when < 0:
                when = 0
            reactor.callLater(when, self.update)

    def _stop(self):
        """
        Stops the game and quits
        """
        self.game.cleanup()
        self._looping_events_check.stop()
        reactor.stop()
