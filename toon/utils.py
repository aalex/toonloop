import osc_protocol

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

