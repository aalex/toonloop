#!/usr/bin/env python
"""
Communication with Pure Data. 
start also the pd patch.
"""
import pprint
import sys

from twisted.internet import reactor
from twisted.internet.error import CannotListenError

from rats import fudi 
from rats import observer

class PureData(observer.Observer):
    """
    ToonLoop API for Pure Data FUDI network messages.

    /frame/add
    /frame/remove
    /clip/select <int>
    /ping
    /pong
    
    Restricted / dangerous ones : 

    /config/set <string> <any>
    /call <string> ...
    /quit
    """
    def __init__(self, **kwargs):
        self.receive_port = 15555
        self.send_port = 17777
        self.send_host = "localhost" # TODO: send to many subscribers.
        self.receiver = None
        self.sender = None
        self.verbose = True
        self.app = None
        self.try_again_delay = 30
        self.try_again = True
        self.manhole_enable = True
        self.__dict__.update(**kwargs)
        self._init_receiver()
        self._init_sender()

    def _init_receiver(self): 
        if self.verbose:
            print "starting FUDI server on port", self.receive_port
        self.receiver = fudi.FUDIServerFactory()
        self.receiver.register_message("/ping", self.ping)
        self.receiver.register_message("/pong", self.pong)
        self.receiver.register_message("/frame/add", self.frame_add)
        self.receiver.register_message("/frame/remove", self.frame_remove)
        self.receiver.register_message("/clip/select", self.frame_remove)
        if self.manhole_enable:
            self.receiver.register_message("/call", self.call)
            self.receiver.register_message("/config/set", self.config_set)
            self.receiver.register_message("/quit", self.quit)
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

    def _on_connected(self, sender):
        if self.verbose:
            print "pd: Connected !"
            print "pd:   sender:", sender
            print "pd:   receiver:", self.receiver
        self.sender = sender
        self.sender.send_message("/ping", 1, 2.0, "bang")
        if self.verbose:
            print "  sent ping"

    def _on_error(self, failure):
        print 'FUDI server unavailable:', self.send_host, self.send_port
        # print "FUDI sender:", failure.getErrorMessage()
        if self.try_again:
            reactor.callLater(self.try_again_delay, self._init_sender)
    
    def _send_message(self, selector, *args):
        # TODO: if cannot send, delete send and raise Error
        if self.sender is None:
            print "pd: Cannot send. Sender is None"
        else:
            try:
                self.sender.send_message(selector, *args)
            except Exception, e:
                print e.message
    # --------------- FUDI Message Callbacks : ----------
    def ping(self, receiver, *args):
        """
        /ping
        """
        if self.verbose:
            print "pd: received ping", args
        response = list(args)
        if self.verbose:
            print "pd: will send", "pong", response
        self._send_message("pong", *response)

    def pong(self, receiver, *args):
        """
        /pong
        """
        if self.verbose:
            print "pd: received pong", args

    def config_set(self, receiver, *args):
        """
        Sets an option
        /options/set
        """
        if self.verbose:
            print "pd: config_set", args
        if self.app is not None:
            try:
                k = args[0]
                v = args[1]
                kind = self.app.config.set(k, v)
                print "      set %s = %s (%s)" % (k, k, kind)
            except Exception, e:
                print e.message

    def frame_add(self, receiver, *args):
        """
        Grabs a frame.
        /frame/add
        """
        if self.verbose:
            print "pd: /frame/add", args
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
            print "pd: /frame/remove", args
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
            print "pd: /clip/select", args
        if self.app is not None:
            try:
                self.app.clip_select(int(args[0]))
            except Exception, e:
                print e.message

    def call(self, receiver, *args):
        """
        Wraps any method from the APP.

        Warning : protect this port !
        """
        if self.app is None:
            print 'ERROR: puredata.app is None'
        else:
            try:
                method = getattr(self.app, args[0])
                if self.verbose:
                    print '      puredata.%s(%s)' % (args[0], str(args[1:])), method
                method(*args[1:])
            except Exception, e:
                print "ERROR @ puredata.call:", e.message
                print sys.exc_info()
    
    def quit(self, receiver, *args):
        """
        /quit
        """
        if self.verbose:
            print "pd: received quit", args
        if self.app is not None:
            self.app.quit()
        else:
            reactor.stop()
        
def start(**kwargs): 
    #receive_port=15555, send_port=17777, send_host='localhost'):
    """
    Factory for the TOonLoOp FUDI service.

    **kwargs
    """
#     kwargs = {
#         'app':app,
#         'receive_port':receive_port,
#         'send_port':send_port,
#         'send_host':send_host
#         } # TODO: send to many subscribers.
#     # kwargs.update(options)
    fudi.VERBOSE = True
    print "Starting PureData"
    pprint.pprint(kwargs)
    
    pd = PureData(**kwargs)
    return pd

if __name__ == '__main__':
    fudi.VERBOSE = True
    pd = PureData()
    reactor.callLater(15, pd._init_sender) 
    reactor.run()

