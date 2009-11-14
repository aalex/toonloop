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
    
    == Sends: ==
     * /toon/frame <i>
     * /toon/sequence <i>
     * /toon/framerate <i>
     * /toon/writehead <i>
     * /sampler/play/start <i>
     * /sampler/play/stop <i>
     * /sampler/record/start <i>
     * /sampler/record/stop <i>

    == Receives: ==
     * /toon/frame/remove
     * /toon/frame/add
     * /toon/osc/subscribe <s> <i>
     * /toon/clip/select <i>
    """
    #TODO: 
    #Receives:
    #/toon/framerate/set <i>
    #/toon/framerate/increase
    #/toon/framerate/decrease
    #/toon/auto/enable <i>
    #/toon/auto/rate <i>
    #/toon/reset
    #/toon/playhead <i>
    #/toon/writehead <i>
    #/toon/clip/reset <i>

    # TODO: UDP broadcasting
    # could be to 255.255.255.255 or 192.168.0.255 or so.
    def __init__(self, toonloop, listen_port=7772, send_port=7770, send_host="localhost"):
        self.toonloop = toonloop
        self.listen_port = listen_port
        self.send_port = send_port
        self.send_host = send_host
        self.osc_server = None
        self.osc_sender = None
        # register callbacks
        self._setup()

    def __del__(self):
        del self.osc_server
        del self.osc_sender

    def _setup(self):
        """
        Starts the server and registers the callbacks.
        """
        self.osc_server = osc.OscServer(self.listen_port)
        self.osc_server.add_callback("/toon/frame/add", None, self._r_frame_add)
        self.osc_server.add_callback("/toon/frame/remove", None, self._r_frame_remove)
        self.osc_server.add_callback("/toon/osc/subscribe", "si", self._r_osc_subscribe) # host, port. Only one subscriber for now.
        self.osc_server.add_callback("/toon/clip/select", "i", self._r_clip_select) 
        self.osc_server.add_callback("/toon/clip/reset", None, self._r_clip_reset) # XXX dangerous
        self.osc_server.add_callback("/toon/clip/save", None, self._r_clip_save)
        self.osc_server.add_callback("/toon/pause", None, self._r_pause)
        self.osc_server.add_callback("/toon/quit", None, self._r_quit) # XXX dangerous
        self.osc_server.add_callback("/toon/config/set", "ss", self._r_config_set) # XXX dangerous
        self.osc_server.add_callback("/toon/print/stats", None, self._r_print_stats) 
        self.osc_server.add_callback("/toon/framerate/increase", "i", self._r_framerate_increase)
        
        self.osc_server.add_callback("/ping", None, self._r_ping)
        self.osc_server.add_callback("/pong", None, self._r_pong)
        self.osc_server.add_callback(None, None, self._r_fallback)
        print("Will now start listening OSC server...")
        self.osc_server.start() # this hangs on Ubuntu 8.10. Update liblo-dev to latest tarball.
        print("OSC Server started. ")
        # the sender
        client_addr = "osc.udp://%s:%d" % (self.send_host, self.send_port)
        self.osc_sender = osc.OscClient(client_addr)
        # would be nice to have a push/subscribe mechanism, with many senders.
        #c.send_message("/ping", ('i', 123), ('f', 3.13159), ('s', "hello"))

    def send_record_start(self, index):
        self._s("/sampler/record/start", index)

    def send_record_stop(self, index):
        self._s("/sampler/record/stop", index)

    def send_play_start(self, index):
        self._s("/sampler/play/start", index)

    def send_play_stop(self, index):
        self._s("/sampler/play/stop", index)
    
    def _s(self, path, index):
        """
        wrapper for sending a osc message with an int arg.
        """
        self.osc_sender.send_message(path, ('i', index))

    def _r_frame_add(self, path, args):
        print("Got /toon/frame/add")
        self.toonloop.frame_add()

    def _r_pause(self, path, args):
        print("Got /toon/pause")
        self.toonloop.pause()

    def _r_config_set(self, path, args):
        key = args[0]
        value = args[1]
        print("Got /toon/config/set %s %s" % (key, value))
        self.toonloop.config_set(key, value)
    
    def _r_clip_select(self, path, args):
        print("Got /toon/clip/select")
        self.toonloop.clip_select(args[0])

    def _r_clip_reset(self, path, args):
        # FIXME : dangerous !!!
        print("Got /toon/clip/reset")
        self.toonloop.clip_reset()

    def _r_framerate_increase(self, path, args):
        dir = args[0]
        print("got /toon/framerate/increase %d" % (dir))
        self.toonloop.framerate_increase(dir)

    def _r_clip_save(self, path, args):
        print("Got /toon/clip/save")
        self.toonloop.clip_save()

    def _r_quit(self, path, args):
        print("Got /toon/quit")
        self.toonloop.quit()
    
    def _r_print_stats(self, path, args):
        print("Got /toon/print/stats")
        self.toonloop.print_stats()

    def _r_frame_remove(self, path, args):
        print("Got /toon/frame/remove")
        self.toonloop.frame_remove()

    def _r_osc_subscribe(self, path, args):
        print("Got /toon/osc/subscribe")
        try:
            self.send_host = args[0]
            self.send_port = args[1]
        except IndexError, e:
            print(e.message)
        else:
            del self.osc_sender
            client_addr = "osc.udp://%s:%d" % (self.send_host, self.send_port)
            print("Now sending to %s" % (client_addr))
            self.osc_sender = osc.OscClient(client_addr)

    def _r_ping(self, path, args):
        print("Got /ping. Sending /pong.")
        self.osc_sender.send_message("/pong")

    def _r_pong(self, path, args):
        print("Got /pong.")

    def _r_fallback(self, path, args, types, src):
        print("got unknown OSC message '%s' from '%s'" % (path, src.get_url()))
        for a, t in zip(args, types):
            print "argument of type '%s': %s" % (t, a)
    
