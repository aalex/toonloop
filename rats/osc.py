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
Implementation of the OSC protocol for Twisted. 

Server and clients use the pyliblo library to integrate well with a twisted 
asynchronous networking application. 
"""
import liblo
import sys
import time

from twisted.internet import reactor 

VERBOSE = True

class OscServerError(Exception):
    """
    Raised when any OSC server error happens.
    """
    pass

class OscClientError(Exception):
    """
    Raised when any OSC client error happens.
    """
    pass

class OscClient(object):
    """
    An OSC client can send OSC messages.
    """
    def __init__(self, port=1234):
        self.port = port
        self._target = None
        self._setup()

    def _setup(self):
        try:
            self._target = liblo.Address(self.port)
        except liblo.AddressError, e:
            raise # OscClientError(str(e)) # TODO: what about the traceback?

    def set_port(self, port):
        """
        Changes the port to send to.
        """
        if port == self.port:
            print "Port is already %d" % (port)
        else:
            self.port = port
            self._setup()

    def send_message(addr, *args):
        """
        See liblo.send : Simply omit the first argument.

        :param addr: OSC address such as /foo/bar
        Other args are arguments types and values.
        
        You can use tuple of (type, value)
        Types are "i", "s", "f", etc.
        """
        liblo.send(self._target, *args)

class OscServer(object):
    """
    An OSC server listens for incoming OSC messages.
    
    The programmer must register callbacks in order to do something with the incoming data.
    """
    def __init__(self, port=1234, interval=0.001):
        """
        :param interval: duration in seconds to wait between each poll
        """
        self.port = port
        self._callbacks = {} # dict of ...
        self.interval = interval
        self._server = None
        self._delayed_id = None
        self.started = False
    
    def start(self):
        """
        Actually start the server.
        """
        self._setup()
    
    def set_port(self, port):
        """
        Used to change the OSC port to listen to.
        """
        if port == self.port:
            print "Port is already %d" % (port)
        else:
            self.port = port
            if self.started:
                self._setup()
        
    def add_callback(self, addr, types, callback, *args):
        """
        Adds a callback for an OSC message. 

        :param addr: URL-like OSC address/pattern
        :param type: string that enumerates the OSC types the method accepts
        :param callback: pointer to a python function or method
        :param args: List of any number of user data.
        """
        self._callbacks[addr] = [types, callback, args]

    def _setup(self):
        """ Called once it is time to start the server or change the port to listen to. """
        global VERBOSE 
        if VERBOSE:
            print "OSC server on port %d"  % (self.port)
        try:
            self._server = liblo.Server(self.port)
        except liblo.ServerError, e:
            raise # OscServerError(str(e)) # TODO: what about the traceback?
        else:
            keys = self._callbacks.keys()
            keys.sort(None, None, True) # reverse so that None key is last
            for key in keys:
                value = self._callbacks[key]
                if VERBOSE:
                    print("Adding OSC method %s %s %s %s" % (key, value[0], value[1], value[2]))
                self._server.add_method(key, value[0], value[1], *value[2])
            self.started = True
            self._delayed_id = reactor.callLater(self.interval, self._poll)

    def _poll(self):
        """ Called often by twisted.reactor to poll the OSC server. """
        self._server.recv(0)
        if self.started:
            # TODO: check if self._delayed_id is not None
            self._delayed_id = reactor.callLater(self.interval, self._poll)

if __name__ == "__main__":
    # examples : 
    def foo_bar_callback(path, args):
        i, f = args
        print "received message '%s' with arguments '%d' and '%f'" % (path, i, f)

    def set_port_callback(path, args, types, src, data):
        i = args[0]
        print "received message '%s' with argument '%d' " % (path, i)
        print "user data : ", data
        data.set_port(i)

    def foo_baz_callback(path, args, types, src, data):
        print "received message '%s'" % path
        print "blob contains %d bytes, user data was '%s'" % (len(args[0]), data)

    def fallback(path, args, types, src):
        print "got unknown message '%s' from '%s'" % (path, src.get_url())
        for a, t in zip(args, types):
            print "argument of type '%s': %s" % (t, a)
    
    s = OscServer(1234)
    s.add_callback("/foo/bar", "if", foo_bar_callback)
    s.add_callback("/set/port", "i", set_port_callback, s) # passing the server itself as user data
    s.add_callback("/foo/baz", "b", foo_baz_callback, "blah")
    s.add_callback(None, None, fallback)
    s.start()
    
    try:
        reactor.run()
    except KeyboardInterrupt:
        print("Exiting.")

