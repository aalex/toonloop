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
Implementation of the OSC protocol for Twisted. 

Server and clients use the pyliblo library to integrate well with a twisted 
asynchronous networking application. 

To use, install liblo from source (not from Ubuntu 8.10's package) and pyliblo.::

 mkdir -p ~/src
 pushd ~/src
 svn co https://liblo.svn.sourceforge.net/svnroot/liblo/trunk liblo 
 pushd liblo
 ./autogen.sh
 make
 sudo make install
 popd
 wget http://das.nasophon.de/download/pyliblo-0.8.0.tar.gz
 tar -zxvf pyliblo-0.8.0.tar.gz
 pushd pyliblo-0.8.0/
 python setup.py  build
 sudo python setup.py install --prefix=/usr/local
 popd
 popd
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
    :param args: one or two arguments. 
    If one, it is either the port number or url. 
    If two, it is the host and the port.
    [host], [port]
    [addr | port]
    """
    def __init__(self, *args):
        self.args = args
        if len(args) == 0:
            args = 1234 # default
        self._target = None
        self._setup()

    def _setup(self):
        try:
            self._target = liblo.Address(*self.args)
        except liblo.AddressError, e:
            raise # OscClientError(str(e)) # TODO: what about the traceback?

    def set_args(self, *args):
        """
        Changes the port|host to send to.
        """
        #if port == self.port:
        #    print "Port is already %d" % (port)
        #else:
        self.args = args
        self._setup()

    def send_message(self, addr, *args):
        """
        See liblo.send : Simply omit the first argument.

        :param addr: OSC address such as /foo/bar
        Other args are arguments types and values.
        
        You can use tuple of (type, value)
        Types are "i", "s", "f", etc.
        """
        liblo.send(self._target, addr, *args)

class OscServer(object):
    """
    An OSC server listens for incoming OSC messages.
    
    The programmer must register callbacks in order to do something with the incoming data.
    """
    def __init__(self, port=1234, interval=0.01):
        """
        :param interval: duration in seconds to wait between each poll
        """
        self.port = port
        self._callbacks = [] # list of lists
        self.interval = interval
        self._server = None
        self._delayed_id = None
        #TODO: self._looping_call = None
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

        What liblo does is to check for matching callbacks in the order they
        were registered, and when a match is found, the callback function is
        called immediately. This means that, if the fallback is to be called
        only when nothing else matches, it needs to be registered last.
        """
        global VERBOSE
        for i in range(len(self._callbacks)):
            if self._callbacks[i][0] == addr:
                if VERBOSE:
                    print("Overriding the previous %s callback." % (addr))
                self._callbacks[i] = [addr, types, callbacks, args]
                return
        # else
        self._callbacks.append([addr, types, callback, args])

    def _setup(self):
        """ Called once it is time to start the server or change the port to listen to. """
        global VERBOSE 
        if VERBOSE:
            print("STarting OSC server on port %d"  % (self.port))
            print("If you experience a long delay while the application is frozen, you might need to upgrade you liblo to the latest version instead of the version packaged with your operating system. Use the latest tarball instead of the .deb.")
        try:
            self._server = liblo.Server(self.port)
        except liblo.ServerError, e:
            raise # OscServerError(str(e)) # TODO: what about the traceback?
        else:
            for callback_list in self._callbacks:
                addr, types, callback, args = callback_list
                if VERBOSE:
                    print("Adding OSC method %s %s %s %s" % (addr, types, callback, args))
                self._server.add_method(addr, types, callback, *args)
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
    
    def ping_callback(path, args):
        i, f, s = args
        print "received message '%s' with arguments '%d', '%f' and '%s'" % (path, i, f, s)
    
    def send_something(client_addr):
        c = OscClient(client_addr)
        c.send_message("/ping", ('i', 123), ('f', 3.13159), ('s', "hello"))

    # UDP broadcasting
    # could be to 255.255.255.255 or 192.168.0.255 or so.
    client_addr = "osc.udp://127.0.0.1:1234"
    # client_addr = "osc.udp://192.168.1.255:1234"
    # client_addr = "osc.udp://255.255.255.255:1234"
    # client_addr = "osc.udp://74.57.167.255:1234"
    s = OscServer(1234)
    s.add_callback("/foo/bar", "if", foo_bar_callback)
    s.add_callback("/set/port", "i", set_port_callback, s) # passing the server itself as user data
    s.add_callback("/foo/baz", "b", foo_baz_callback, "blah")
    s.add_callback("/ping", "ifs", ping_callback)
    s.add_callback(None, None, fallback)
    print("Will now start listening OSC server...")
    s.start() # this hangs on Ubuntu 8.10. Update liblo-dev to latest tarball.
    print("Server started. ")
    reactor.callLater(1.0, send_something, client_addr)

    try:
        reactor.run()
    except KeyboardInterrupt:
        print("Exiting.")

