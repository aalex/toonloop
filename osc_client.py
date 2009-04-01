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
"""
OSC test client.
"""
import os
import sys
import time

from twisted.internet import reactor

from toon import osc_protocol 
from toon import osc_create_and_send


class TestClient:
    """
    OSC callbacks and main logic
    """
    def ping(self, addr, tags, msg, host):
        """
        /ping
        
        answers /pong
        args: pattern, tags, data, (self.client_host, self.client_port)
        """
        print addr, tags, msg, host
        print "Received /ping. Sending /pong"
        osc_create_and_send(self.osc, (host[0], self.send_port), "/pong")
    
    def pong(self, addr, tags, stuff, host):
        print "received pong from", host
        print "Roundtrip : %f ms." % ((time.time() - self._ping_time) * 1000)
        
    def send_ping(self):
        self._ping_time = time.time()
        osc_create_and_send(self.osc, (self.remote_host, self.send_port), '/ping')
    
    def __init__(self):
        self.send_port = 4444
        self.receive_port = 3333
        self.remote_host = '127.0.0.1'
        self.osc  = osc_protocol.Osc()
        self._ping_time = 0

        print 'starting OSC server on port ', self.receive_port
        try:
            reactor.listenUDP(self.receive_port, self.osc)
        except Exception,e:
            print e
        # ---------- ping / pong
        self.osc.add_msg_handler("/ping", self.ping)
        self.osc.add_msg_handler("/pong", self.pong)
        
        print 'OSC callbacks:'
        for c in self.osc.callbacks:
            print c
    
if __name__ == '__main__':
    app = TestClient()
    reactor.callLater(0, app.send_ping)
    reactor.run()
    
