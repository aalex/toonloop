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

osc_protocol.py is part of rawmaterials/Propulse[art] from SAT.
See https://svn.sat.qc.ca/trac/rawmaterials
"""

from twisted.internet import reactor
from rats import osc_protocol
from rats.observer import Observer

def _print(text):
    """
    Simply prints to the console debug messages
    """
    print "\n", text

def osc_create_and_send(osc, host, addr="", msg=None, typehint=None):
    """
    Creates and send and OSC message.
    
    :param osc: is a OSC twisted server/client instance. see osc_protocol.py
    :param host: is a tuple (ip, port)
    :param addr: is the 4th arg to OSC callbacks
    :param msg: can be a list

    Example:
    osc_create_and_send(self.osc,host,'/foo/bar', [1,2,'egg','spam'])
    """
    m = osc_protocol.OscMessage(addr)
    if msg is not None:
        m.append(msg, typehint)
    osc.send_message(host[0], host[1], m)

class ToonOsc(Observer):
    """
    OSC callbacks and sends for ToonLoop
    
    Sends:
    /toon/frame <i>
    /toon/sequence <i>
    /toon/framerate <i>
    /toon/writehead <i>
    
    Receives:
    /toon/framerate/set <i>
    /toon/framerate/increase
    /toon/framerate/decrease
    /toon/auto/enable <i>
    /toon/auto/rate <i>
    /toon/osc/send/host <s>
    /toon/osc/send/port <i>
    /toon/sequence <i>
    /toon/frame/delete
    /toon/frame/add
    /toon/reset
    /toon/playhead <i>
    /toon/writehead <i>
    """
    def r_ping(self, addr, tags, msg, host):
        """
        /ping
        
        answers /pong
        args: pattern, tags, data, (self.client_host, self.client_port)
        """
        print addr, tags, msg, host
        _print("Received /ping. Sending /pong")
        osc_create_and_send(self.osc, (host[0], self.send_port), "/pong")
    
    def r_pong(self, addr, tags, stuff, host):
        print "received pong from", host

    def r_frame_add(self, addr, tags, stuff, host):
        self.toonloop.grab_image()
        print "received /frame/add from", host

    def __init__(self, toonloop):
        self.toonloop = toonloop
        self.send_port = 3333
        self.receive_port = 4444
        self.send_host = 'localhost'
        self.osc = osc_protocol.Osc()
        try:
            print "OSC listening on port %d" % (self.receive_port)
            reactor.listenUDP(self.receive_port, self.osc)
        except CannotListenError,e:
            _print("ERROR: port already in use : %d" % (port_num))
            _print("Please quit and try again.")
            _print(e)
        
        # /ping
        self.osc.add_msg_handler('/ping', self.r_ping)
        self.osc.add_msg_handler('/pong', self.r_pong)
        self.osc.add_msg_handler('/frame/add', self.r_frame_add)

        print 'OSC callbacks:'
        for c in self.osc.callbacks:
            print c

