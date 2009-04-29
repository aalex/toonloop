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
    def __init__(self, **kwargs):
        self.receive_port = 15555
        self.send_port = 17777
        self.send_host = "localhost" # TODO: send to many subscribers.
        self.receiver = None
        self.sender = None
        self.verbose = True
        self.app = None
        self.__dict__.update(**kwargs)
        
        self._init_receiver()
        self._init_sender()

    def _init_receiver(self): 
        if self.verbose:
            print "starting FUDI server on port", self.receive_port
        self.receiver = fudi.FUDIServerFactory()
        self.receiver.register_message("/ping", self.ping)
        self.receiver.register_message("/quit", self.quit)
        self.receiver.register_message("/pong", self.pong)
        self.receiver.register_message("/config/set", self.config_set)
        self.receiver.register_message("/frame/add", self.frame_add)
        self.receiver.register_message("/frame/remove", self.frame_remove)
        self.receiver.register_message("/clip/select", self.frame_remove)
        self.receiver.register_message("/call", self.call)
        try:
            reactor.listenTCP(self.receive_port, self.receiver)
        except CannotListenError, e:
            print e.message
            raise
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
        reactor.callLater(10, self._init_sender)
    
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

    def config_set(self, receiver, *args):
        """
        Sets an option
        /options/set
        """
        if self.verbose:
            print "config_set", args
        if self.app is not None:
            try:
                k = args[0]
                v = args[1]
                kind = self.app.config.set(k, v)
                print "set %s = %s (%s)" % (k, k, kind)
            except Exception, e:
                print e.message

    def frame_add(self, receiver, *args):
        """
        Grabs a frame.
        /frame/add
        """
        if self.verbose:
            print "/frame/add", args
        if self.app is not None:
            try:
                self.app.frame_add()
            except Exception, e:
                print e.message
    
    def frame_remove(self, receiver, *args):
        """
        Grabs a frame.
        /frame/add
        """
        if self.verbose:
            print "/frame/remove", args
        if self.app is not None:
            try:
                self.app.frame_remove()
            except Exception, e:
                print e.message

    def clip_select(self, receiver, *args): 
        """
        /clip select <index>
        index=0
        """
        if self.verbose:
            print "/clip/select", args
        if self.app is not None:
            try:
                self.app.clip_select(int(args[0]))
            except Exception, e:
                print e.message

    def call(self, receiver, *args):
        """
        Wraps any method from the APP.
        """
        if self.verbose:
            print "ANY:", args
        if self.app is not None:
            try:
                getattr(self.app, args[0])(*args[1:])
                print 'app.%s(%s)' % (args[0], args[1:])
            except Exception, e:
                print e.message

    
    def quit(self, receiver, *args):
        """
        /quit
        """
        if self.verbose:
            print "received quit", args
        if self.app is not None:
            self.app.quit()
        else:
            reactor.stop()
        
def start(app):
    """
    **kwargs
    """
    fudi.VERBOSE = True
    kwargs = {
        'app':app,
        'receive_port':15555,
        'send_port':17777,
        'send_host':"localhost"
        } # TODO: send to many subscribers.
    # kwargs.update(options)
    pd = PureData(**kwargs)
    return pd

if __name__ == '__main__':
    fudi.VERBOSE = True
    pd = PureData()
    reactor.callLater(15, pd._init_sender) 
    reactor.run()
