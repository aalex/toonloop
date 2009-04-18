import types

from twisted.internet import reactor
from twisted.internet.protocol import Protocol
from twisted.internet.protocol import ClientCreator
from twisted.protocols import basic
from twisted.internet.protocol import Factory
from twisted.internet.protocol import ClientFactory
from twisted.python import log

VERBOSE = True

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
            print "data:", data
        try:
            message = data.split(";")[0].strip()
        except KeyError:
            log.msg("Got a line without trailing semi-colon.")
        else:
            if VERBOSE:
                print "message:", message
            atoms = message.split()
            if len(atoms) > 0:
                output = []
                selector = atoms[0]
                for atom in atoms[1:]:
                    atom = atom.strip()
                    if VERBOSE:
                        print "> atom:", atom
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
                        print "Calling :", selector, output
                    try:
                        self.factory.callbacks[selector](self, output)
                    except TypeError, e:
                        print e.message
                else:
                    #log.msg("Invalid selector %s." % (selector))
                    print "Invalid selector %s." % (selector)

    def send_message(self, *data):
        """
        Converts int, float, string to FUDI atoms and sends them.
        :param data: list of basic types variables.
        """
        txt = ""
        for atom in data:
            txt += "%s " % (atom)
        txt = txt.strip() + ";\r\n"
        print "sending", txt
        self.transport.write(txt)

class FUDIServerFactory(Factory):
    """
    You should attach callbacks to instances of this class.
    """
    protocol = FUDIProtocol
    def __init__(self):
        self.callbacks = {}

    def register_message(self, selector, callback):
        if type(callback) not in (types.FunctionType, types.MethodType):
            raise TypeError("Callback '%s' is not callable" % repr(callback))    
        self.callbacks[selector] = callback

def create_FUDI_client(host, port):
    deferred = ClientCreator(reactor, FUDIProtocol).connectTCP(host, port)
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
