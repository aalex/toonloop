#!/usr/bin/env python
#
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
# You should have received a copy of the gnu general public license
# along with ToonLoop.  If not, see <http://www.gnu.org/licenses/>.
#

import random
import sys
from time import strftime
import os

from twisted.internet import reactor

import osc_protocol 
from utils import osc_create_and_send

def _print(text):
    print "\n", text

class FxOsc(object):
    """
    OSC callbacks and send to control FxLoop from Python
    
    Sends:
    ping
    /toon/frame <i>
    /toon/sequence <i>
    /toon/framerate <i>
    /toon/writehead <i>
    
    Receives:
    pong
    """

    def __init__(self):
        self.send_port = 17777
        self.receive_port = 88888
        self.send_host = '127.0.0.1'
        self.osc = osc_protocol.Osc()

        try:
            print "OSC listening on port %d" % (self.receive_port)
            reactor.listenUDP(self.receive_port, self.osc)
        except CannotListenError,e:
            _print("ERROR: port already in use : %d" % (port_num))
            _print("Please quit and try again.")
            _print(e)
        
        # /ping
        self.osc.add_msg_handler('/ping', self.ping)
        self.osc.add_msg_handler('/pong', self.pong)

        print 'OSC callbacks:'
        for c in self.osc.callbacks:
            print c

    def ping(self, addr, tags, msg, host):
        """
        /ping
        
        answers /pong
        args: pattern, tags, data, (self.client_host, self.client_port)
        """
        print addr, tags, msg, host
        _print("Received /ping. Sending /pong")
        osc_create_and_send(self.osc, (host[0], self.send_port), "/pong")
    
    def pong(self,addr, tags, stuff, host):
        print "received pong from", host


    def send_ping(self):
        _print("Sending /ping")
        osc_create_and_send(self.osc, (self.send_host, self.send_port), "/ping")

    def send_random_note(self):
        note = random.uniform(0, 127)
        amplitude = 1.0
        _print("rand note %d %f" % (note, amplitude))
        osc_create_and_send(self.osc, (self.send_host, self.send_port), "/test/note", [note, amplitude])
        reactor.callLater(1.0, self.send_random_note)

class FxError(Exception):
    """
    Any error ToonLoop might encouter
    """
    pass

if __name__ == "__main__":
    """
    Starts the application
    """
    random.seed()
    fxloop = FxOsc()
    reactor.callLater(1.0, fxloop.send_random_note)
    reactor.callLater(0.0, fxloop.send_ping)
    try:
        reactor.run()
    except KeyboardInterrupt:
        pass
    print "\nExiting toonloop"
