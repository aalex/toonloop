#!/usr/bin/env python
"""
Communication with Pure Data. 
start also the pd patch.
"""
from twisted.internet import reactor
from toon import fudi 
from rats import observer
from twisted.internet.error import CannotListenError

class PureData(observer.Observer):
    def __init__(self):
        self.receive_port = 15555
        self.send_port = 17777
        self.send_host = "localhost" # TODO: send to many subscribers.
        self.receiver = None
        self.sender = None
        self.verbose = True
        
        self._init_receiver()
        self._init_sender()

    def _init_receiver(self): 
        if self.verbose:
            print "starting FUDI server on port", self.receive_port
        self.receiver = fudi.FUDIServerFactory()
        self.receiver.register_message("/ping", self.ping)
        self.receiver.register_message("/quit", self.quit)
        self.receiver.register_message("/pong", self.pong)
        try:
            reactor.listenTCP(self.receive_port, self.receiver)
        except CannotListenError, e:
            print e.message
            self.receiver = None

    def _init_sender(self):
        deferred = fudi.create_FUDI_client(self.send_host, self.send_port)
        deferred.addCallback(self._on_connected)
        deferred.addErrback(self._on_error)

    def _send_message(self, selector, *args):
        # TODO: if cannot send, delete send and raise Error
        if self.sender is None:
            print "Cannot send. Sender is None"
        else:
            try:
                self.sender.send_message(selector, *args)
            except Exception, e:
                print e.message

    def _on_connected(self, sender):
        if self.verbose:
            print "Connected !"
            print "  sender:", sender
            print "  receiver:", self.receiver
        self.sender = sender
        self.sender.send_message("/ping", 1, 2.0, "bang")
        if self.verbose:
            print "  sent ping"

    def _on_error(self, failure):
        print "FUDI sender:", failure.getErrorMessage()
    
    # --------------- FUDI Message Callbacks : ----------
    def ping(self, receiver, *args):
        """
        /ping
        """
        if self.verbose:
            print "received ping", args
        response = list(args)
        if self.verbose:
            print "will send", "pong", response
        self._send_message("pong", *response)

    def pong(self, receiver, *args):
        """
        /pong
        """
        if self.verbose:
            print "received pong", args

    def quit(self, receiver, *args):
        """
        /quit
        """
        if self.verbose:
            print "received quit", args
        reactor.stop()
        
if __name__ == '__main__':
    fudi.VERBOSE = True
    pd = PureData()
    reactor.callLater(15, pd._init_sender) 
    reactor.run()
