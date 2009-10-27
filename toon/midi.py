#!/usr/bin/env python
"""
Contains an example of midi input
"""
import sys
import os

import pygame
import pygame.midi
from pygame.locals import *

from twisted.internet import reactor
from twisted.internet import task


class TwistedMidiInput(object):
    def __init__(self, device_id=None):
        self.device_id = device_id
        self.midi_input = None
        self._setup_input()
        self._looping_call = task.LoopingCall(self.poll)
        
    def _print_devices_info(self, input_only=True):
        """
        Prints infos on all MIDI input devices.
        """
        for i in range( pygame.midi.get_count() ):
            r = pygame.midi.get_device_info(i)
            (interf, name, input, output, opened) = r
            in_out = ""
            if input:
                in_out = "(input)"
            if output:
                in_out = "(output)"
            if input_only is False or input:
                print ("%2i: interface :%s:, name :%s:, opened :%s:  %s" %
                       (i, interf, name, opened, in_out))

    def _setup_input(self):
        pygame.fastevent.init()
        #event_get = pygame.fastevent.get
        pygame.midi.init()
        self._print_devices_info()
        print("MIDIIN event : %s" % (str(pygame.midi.MIDIIN)))
        if self.device_id is None:
            input_id = pygame.midi.get_default_input_id()
        else:
            input_id = self.device_id
        print ("using input_id :%s:" % (input_id))
        self.midi_input = pygame.midi.Input(input_id)
        pygame.display.set_mode((1, 1))
    
    def start(self):
        """
        Starts polling
        Returns a deferred whose callback will be invoked with self when self.stop is called, or whose errback will be invoked when the function raises an exception or returned a deferred that has its errback invoked. 
        """
        interval = 0.05
        return self._looping_call.start(interval)

    def poll(self):
        event_post = pygame.fastevent.post
        #try:
            #while going:
            #    events = event_get()
            #    for e in events:
            #        if e.type in [QUIT]:
            #            going = False
            #        if e.type in [KEYDOWN]:
            #            going = False
            #        if e.type in [pygame.midi.MIDIIN]:
            #            print (e)
        if self.midi_input.poll():
            midi_events = self.midi_input.read(10)
            # convert them into pygame events.
            midi_evs = pygame.midi.midis2events(midi_events, self.midi_input.device_id)
            for midi_ev in midi_evs:
                print("GOT MIDI EVENT: %s" % (midi_ev))
                #event_post(midi_ev)

    def __del__(self):
        del self.midi_input
        pygame.midi.quit()

if __name__ == '__main__':
    try:
        device_id = int(sys.argv[-1])
    except:
        device_id = None
    pygame.init()
    midi_input_manager = TwistedMidiInput(device_id)
    midi_input_manager.start()
    try:
        reactor.run()
    except KeyboardInterrupt:
        print "quit."

