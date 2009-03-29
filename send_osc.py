#!/usr/bin/env python
#
# ToonLoop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# http://toonloop.com
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
Utility to send a OSC message using the command line.
"""
import liblo
import sys
from optparse import OptionParser
__version__ = '1.0 alpha'

if __name__ == '__main__':
    osc_addr = '/ping' # could be /frame/add
    osc_args = []
    send_host = 'locahost'
    send_port = 4444
    verbose = False
    
    # command line parsing
    usage = "%prog [arguments] [OSC address] [OSC arguments]\n"
    usage += "    example: %prog /frame/add"
    parser = OptionParser(usage=usage, version=str(__version__))
    parser.add_option("-p", "--port", default=4444, type="int", \
        help="Specifies the OSC port to send to.")
    parser.add_option("-H", "--host", type="string", default="localhost", \
        help="Specifies a hostname to send to.")
    parser.add_option("-v", "--verbose", dest="verbose", action="store_true", \
        help="Sets the output to be verbose.")
    (options, args) = parser.parse_args()
    if options.verbose:
        verbose = True
    if options.port:
        send_port = options.port
    if options.host:
        send_host = options.host
    if len(args) > 0:
        osc_addr = args[0]
    if len(args) > 1:
        osc_args = args[1:]
    try:
        target = liblo.Address(send_port)
    except liblo.AddressError, err:
        print "Problem with OSC port number %s : %s" % (send_port, str(err))
        sys.exit(1)
    msg = liblo.Message(osc_addr)
    for arg in osc_args:
        msg.add(arg)
    liblo.send(target, msg)

