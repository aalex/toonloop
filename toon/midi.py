#!/usr/bin/env python
"""
MIDI input using pygame and twisted.
"""
import sys
import os

import pygame
import pygame.midi
from pygame.locals import *

from twisted.internet import reactor
from twisted.internet import task

class NotConnectedError(Exception):
    """
    Raised when trying to start the simple midi input when not connected.
    """
    pass

class SimpleMidiInput(object):
    """
    MIDI input using pygame.midi.
    """
    def __init__(self, input_id=None, verbose=True):
        if input_id == -1:
            input_id = None # -1 is an alias for None
        self.input_id = input_id
        self.midi_input = None
        self.is_connected = False
        self.verbose = verbose
        self._use_dummy_display = False # for tests.
        self.convert_to_pygame_event = False
        self.callbacks = []
        
        # initiate MIDI
        self._setup_input()
        self._looping_call = task.LoopingCall(self.poll)
        
    def print_devices_info(self, input_only=True):
        """
        Prints infos on all MIDI input devices.
        """
        if input_only:
            print("MIDI input devices:")
        for i in range(pygame.midi.get_count()):
            r = pygame.midi.get_device_info(i)
            (interf, name, input, output, opened) = r
            in_out = ""
            if input:
                in_out = "(input)"
            if output:
                in_out = "(output)"
            if input_only is False or input:
                print("%2i: interface :%s:, name :%s:, opened :%s:  %s" % (i, interf, name, opened, in_out))

    def _verb(self, txt):
        """
        Prints INFO messages
        """
        if self.verbose:
            print(txt)
    
    def _setup_input(self):
        if self.convert_to_pygame_event:
            pygame.fastevent.init()
        pygame.midi.init()
        #if self.verbose:
        self.print_devices_info()
        self._verb("MIDIIN event : %s" % (str(pygame.midi.MIDIIN)))
        if self.input_id is None:
            self.input_id = pygame.midi.get_default_input_id()
        self._verb("Using input_id :%s:" % (self.input_id))
        try:
            self.midi_input = pygame.midi.Input(self.input_id)
            self.is_connected = True
        except pygame.midi.MidiException, e:
            print(e.message)
            self.is_connected = False
        if self._use_dummy_display: # for tests
            pygame.display.set_mode((1, 1))
    
    def start(self):
        """
        Starts polling
        Returns a deferred whose callback will be invoked with self when self.stop is called, 
        or whose errback will be invoked when the function raises an exception or returned a 
        deferred that has its errback invoked. 
        """
        if self.is_connected:
            interval = 0.05
            return self._looping_call.start(interval)
        else:
            raise NotConnectedError("We are not connected to any MIDI device !")

    def poll(self):
        #try:
            #while going:
            #    if self.convert_to_pygame_event:
            #    events = pygame.fastevent.get()
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
            midi_events = pygame.midi.midis2events(midi_events, self.midi_input.device_id)
            for midi_event in midi_events:
                self._verb("GOT MIDI EVENT: %s" % (midi_event))
                self._call_callbacks(midi_event)
                if self.convert_to_pygame_event:
                    pygame.fastevent.post(midi_event)
    
    def register_callback(self, callback):
        if callback not in self.callbacks:
            self.callbacks.append(callback)

    def _call_callbacks(self, event):
        """
        Called when a MIDI event occurs.
        """
        for cb in self.callbacks:
            cb(event)

    def __del__(self):
        del self.midi_input
        pygame.midi.quit()

if __name__ == '__main__':
    def _cb(event):
        if event.status == 144: # MIDI note
            print("MIDI note: Pitch: %s Velocity: %s" % (event.data1, event.data2))
        elif event.status == 176: # MIDI control
            print("MIDI control: ID: %s Value: %s" % (event.data1, event.data2))

    try:
        device_id = int(sys.argv[-1])
    except:
        device_id = None
    pygame.init()
    midi_input_manager = SimpleMidiInput(device_id)
    midi_input_manager.register_callback(_cb)
    midi_input_manager.start()
    try:
        reactor.run()
    except KeyboardInterrupt:
        print "quit."

