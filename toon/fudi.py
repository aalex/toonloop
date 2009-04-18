import types

from twisted.internet import reactor
from twisted.internet.protocol import Protocol
from twisted.protocols import basic
from twisted.internet.defer import Deferred
from twisted.internet.protocol import Factory
from twisted.internet.protocol import ClientFactory
from twisted.python import log

class FUDIProtocol(basic.LineReceiver):
    """
    FUDI protocol implementation in Python.
    
    Simple ASCII based protocol from Miller Puckette for Pure Data.
    """
    def lineReceived(self, data):
        try:
            messages = data.split(";")[0].strip()
        except KeyError:
            log.msg("Got a line without trailing semi-colon.")
        else:
            for message in messages:
                atoms = message.split()
                if len(atoms) > 0:
                    output = []
                    selector = atoms[0]
                    for atom in atoms[1:]:
                        if atom.isdigit():
                            output.append(int(atom))
                        else:
                            try:
                                val = float(atom)
                                output.append(atom)
                            except ValueError:
                                output.append(str(atom))
                    if self.factory.callbacks.has_key(selector):
                        self.factory.callbacks[selector](output)
                    else:
                        log.msg("Invalid selector %s." % (selector))

    def send_message(self, data):
        """
        Converts int, float, string to FUDI atoms and sends them.
        :param data: list of basic types variables.
        """
        txt = ""
        for atom in data:
            txt += "%s" % (atom)
        data += ";\r\n"
        self.transport.write(txt)

class FUDIServerFactory(Factory):
    """
    You should attach callbacks to instances of this class.
    """
    protocol = FUDIProtocol
    def __init__(self):
        self.callbacks = {}

    def register_callback(self, selector, callback):
        if type(callback) not in (types.FunctionType, types.MethodType):
            raise TypeError("Callback '%s' is not callable" % repr(callback))    
        self.callbacks[selector] = callback

class FUDIClientFactory(ClientFactory):
    protocol = FUDIProtocol
    def __init__(self):
        #self.deferred = Deferred()

    def clientConnectionFailed(self, _, reason):
        print 'Connection failed. Reason:', reason
        #self.deferred.errback(reason)

    def startedConnecting(self, connector):
        print 'Started to connect.'
    
    def clientConnectionLost(self, connector, reason):
        print 'Lost connection.  Reason:', reason

if __name__ == "__main__":
    def ping(*args):
        print "received ping", args
        #s.send_message("pong", 1, 2, 3.0)

    # def pong(*args):
    #     print "received ping", args
    #     reactor.stop()

    s = FUDIServerFactory()
    s.register_callback("ping", ping)
    reactor.listenTCP(15555, s)
    
    c = FUDIClientFactory()
    reactor.connectTCP('localhost', 15555, c)
    
    reactor.callLater(0, c.send_message, "ping", 4, 5, 6)
    reactor.run()
