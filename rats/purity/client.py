#!/usr/bin/env python
#
# ToonLoop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# <alexandre@quessy.net> & <le.businessman@gmail.com>
# http://www.toonloop.com
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
Simpler FUDI sender.

"""
import sys
from twisted.internet import reactor
from twisted.internet import defer
from rats import fudi
from rats.purity import server

class PurityClient(object):
    """
    Dynamic patching Pure Data message sender.
    Used for dynamic patching with Pd.
    """
    # TODO: connect directly to pd-gui port, which is 5400 + n
    def __init__(self, receive_port=14444, send_port = 15555, use_tcp=True, quit=quit):
        self.send_port = send_port
        self.receive_port = receive_port
        self.client_protocol = None
        self.fudi_server = None
        self.use_tcp = use_tcp # TODO
        self.quit = quit

    def server_start(self):
        """ returns server """
        self.fudi_server = fudi.FUDIServerFactory()
        self.fudi_server.register_message("pong", self.pong)
        reactor.listenTCP(self.receive_port, self.fudi_server)
        return self.fudi_server

    def client_start(self):
        """ 
        Starts sender. 
        returns deferred """
        self.client_protocol = None
        print("Starting FUDI client sending to %d" % (self.send_port))
        deferred = fudi.create_FUDI_client('localhost', self.send_port, self.use_tcp)
        deferred.addCallback(self.on_client_connected)
        deferred.addErrback(self.on_client_error)
        return deferred

    def pong(self, protocol, *args):
        """ Receives FUDI pong """
        print "received pong", args
        # print("stopping reactor")
        # reactor.stop()

    def on_client_connected(self, protocol):
        """ Client can send messages to Pure Data """
        self.client_protocol = protocol
        # self.client_protocol.send_message("ping", 1, 2.0, "bang")
        # print "sent ping"
        return protocol # pass it to the next

    def on_client_error(self, failure):
        """ Client cannot send data to pd """
        print "Error trying to connect.", failure
        raise Exception("Could not connect to pd.... Dying.")
        # print "stop"
        # reactor.stop()


    def send_message(self, selector, *args):
        """ Send a message to pure data """
        if self.client_protocol is not None:
            # if fudi.VERBOSE:
            # print("sending %s" % (str(args)))
            # print args[0], args[1:]
            # args = list(args[1:])
            # atom = args[0]
            # print("will send %s %s" % (selector, args))
            # self.client_protocol.send_message(*args, selector)
            self.client_protocol.send_message(selector, *args)
        else:
            print("Could not send %s" % (str(args)))
        if self.quit:
            print "stopping the application"
            # TODO: try/catch
            reactor.callLater(0, reactor.stop)

def create_simple_client():
    """
    Creates a purity server (Pure Data process manager)
    and a purity client. 
    """
    # TODO: receive message from pd to know when it is really ready.
    def _callback(protocol, deferred1, client):
        deferred1.callback(client)
        return True
    def _later(deferred1, client):
        # time.sleep(1.0) # Wait until pd is ready. #TODO: use netsend instead.
        deferred2 = client.client_start() # start it
        deferred2.addCallback(_callback, deferred1, client)
    deferred = defer.Deferred()
    pid = server.fork_and_start_pd()
    if pid != 0:
        the_client = PurityClient(
            receive_port=15555, 
            send_port=17777, 
            quit=False, 
            use_tcp=True) # create the client
        reactor.callLater(5.0, _later, deferred, the_client)
    else:
        sys.exit(0) # do not do anything else here !
    return deferred



