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
OSC controls for ToonLoop
"""
from rats import osc

def _print(text):
    """
    Simply prints to the console debug messages
    """
    print(text)

class ToonOsc(object):
    """
    OSC callbacks and sends for ToonLoop
    
    Sends:
    /toon/frame <i>
    /toon/sequence <i>
    /toon/framerate <i>
    /toon/writehead <i>
    /sampler/play/start <i>
    /sampler/play/stop <i>
    /sampler/record/start <i>
    /sampler/record/stop <i>
    """
    #TODO: 
    #Receives:
    #/toon/framerate/set <i>
    #/toon/framerate/increase
    #/toon/framerate/decrease
    #/toon/auto/enable <i>
    #/toon/auto/rate <i>
    #/toon/osc/send/host <s>
    #/toon/osc/send/port <i>
    #/toon/sequence <i>
    #/toon/frame/delete
    #/toon/frame/add
    #/toon/reset
    #/toon/playhead <i>
    #/toon/writehead <i>

    def __init__(self, toonloop, listen_port=12346, send_port=12345, send_host="localhost"):
        self.toonloop = toonloop
        self.listen_port = listen_port
        self.send_port = send_port
        self.send_host = send_host
        self.osc_server = None
        self.osc_sender = None
        # register callbacks
        self._setup()

    def _setup(self):
        """
        Starts the server and registers the callbacks.
        """
        self.osc_server = osc.OscServer(self.listen_port)
        self.osc_server.add_callback("/frame/add", None, self.cb_frame_add)
        self.osc_server.add_callback("/ping", None, self.cb_ping)
        self.osc_server.add_callback("/pong", None, self.cb_pong)
        self.osc_server.add_callback(None, None, self.cb_fallback)
        print("Will now start listening OSC server...")
        self.osc_server.start() # this hangs on Ubuntu 8.10. Update liblo-dev to latest tarball.
        print("OSC Server started. ")
        # the sender
        client_addr = "osc.udp://%s:%d" % (self.send_host, self.send_port)
        self.osc_sender = osc.OscClient(client_addr)
        #c.send_message("/ping", ('i', 123), ('f', 3.13159), ('s', "hello"))

    # UDP broadcasting
    # could be to 255.255.255.255 or 192.168.0.255 or so.

    def send_record_start(self, index):
        self._s("/sampler/record/start", index)

    def send_record_stop(self, index):
        self._s("/sampler/record/stop", index)

    def send_play_start(self, index):
        self._s("/sampler/play/start", index)

    def send_play_stop(self, index):
        self._s("/sampler/play/stop", index)
    
    def _s(self, path, index):
        self.osc_sender.send_message(path, ('i', index))
        

    def cb_frame_add(self, path, args):
        self.toonloop.frame_add()
        print("/frame/add")

    def cb_ping(self, path, args):
        print("/ping")

    def cb_pong(self, path, args):
        print("/pong")

    def cb_fallback(self, path, args, types, src):
        print("got unknown OSC message '%s' from '%s'" % (path, src.get_url()))
        for a, t in zip(args, types):
            print "argument of type '%s': %s" % (t, a)
    

