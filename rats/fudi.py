#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# The Purity library for Pure Data dynamic patching.
#
# Copyright 2009 Alexandre Quessy
# <alexandre@quessy.net>
# http://alexandre.quessy.net
#
# Purity is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Purity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with Purity.  If not, see <http://www.gnu.org/licenses/>.
#
"""
FUDI protocol implementation in Python using Twisted.

Simple ASCII based protocol from Miller Puckette for Pure Data.
"""

import types

from twisted.internet import reactor
from twisted.internet.protocol import Protocol
from twisted.internet.protocol import ClientCreator
from twisted.internet.protocol import Factory
from twisted.internet.protocol import ClientFactory
from twisted.protocols import basic
from twisted.python import log

VERBOSE = False

def to_fudi(selector, *atoms):
    """
    Converts int, float, string to FUDI atoms string.
    :param data: list of basic types variables.
    Public FUDI message converter
    """
    # if VERBOSE:
    #     print "FUDI: to_fudi", selector, atoms
    txt = str(selector)
    for atom in atoms:
        txt = txt + " %s" % (atom)
    txt = txt + " ;\r\n"
    return txt

class FUDIProtocol(basic.LineReceiver):
    """
    FUDI protocol implementation in Python.
    
    Simple ASCII based protocol from Miller Puckette for Pure Data.
    """
    #def connectionMade(self):
    #    print "connection made", self.transport# , self.factory

    delimiter = ';'

    def lineReceived(self, data):
        if VERBOSE:
            print "FUDI: data:", data
        try:
            message = data.split(";")[0].strip()
        except KeyError:
            log.msg("Got a line without trailing semi-colon.")
        else:
            if VERBOSE:
                print "FUDI: message:", message
            atoms = message.split()
            if len(atoms) > 0:
                output = []
                selector = atoms[0]
                for atom in atoms[1:]:
                    atom = atom.strip()
                    if VERBOSE:
                        print "FUDI: > atom:", atom
                    if atom.isdigit():
                        output.append(int(atom))
                    else:
                        try:
                            val = float(atom)
                            output.append(atom)
                        except ValueError:
                            output.append(str(atom))
                if self.factory.callbacks.has_key(selector):
                    if VERBOSE:
                        print "FUDI: Calling :", selector, output
                    try:
                        self.factory.callbacks[selector](self, *output)
                    except TypeError, e:
                        print "FUDI:lineReceived():", e.message
                else:
                    #log.msg("Invalid selector %s." % (selector))
                    print "FUDI: Invalid selector %s." % (selector)

    def send_message(self, selector, *atoms):
        """
        Converts int, float, string to FUDI atoms and sends them.
        :param data: list of basic types variables.
        """
        if VERBOSE:
            print "send_message", selector, atoms
        txt = to_fudi(selector, *atoms)
        if VERBOSE:
            print "FUDI: sending", txt
        self.transport.write(txt)

class FUDIServerFactory(Factory):
    """
    Factory for FUDI receivers.
    
    You should attach FUDI message callbacks to an instance of this.
    """
    protocol = FUDIProtocol
    def __init__(self):
        self.callbacks = {}

    def register_message(self, selector, callback):
        if type(callback) not in (types.FunctionType, types.MethodType):
            raise TypeError("Callback '%s' is not callable" % repr(callback))    
        self.callbacks[selector] = callback

def create_FUDI_client(host, port, tcp=True):
    """
    Creates a FUDI sender.

    When connected, will call its callbacks with the sender instance.
    :return: deferred instance
    """
    if tcp:
        deferred = ClientCreator(reactor, FUDIProtocol).connectTCP(host, port)
    else:
        deferred = ClientCreator(reactor, FUDIProtocol).connectUDP(host, port)
    return deferred

if __name__ == "__main__":
    VERBOSE = True

    def ping(protocol, *args):
        print "received ping", args
        reactor.stop()

    def on_connected(protocol):
        protocol.send_message("ping", 1, 2.0, "bang")
        print "sent ping"

    def on_error(failure):
        print "Error trying to connect.", failure
        reactor.stop()
        
    PORT_NUMBER = 15555

    s = FUDIServerFactory()
    s.register_message("ping", ping)
    reactor.listenTCP(PORT_NUMBER, s)
    
    create_FUDI_client('localhost', PORT_NUMBER).addCallback(on_connected).addErrback(on_error)
    reactor.run()
