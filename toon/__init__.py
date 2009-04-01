#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# ToonLoop
# Copyright (C) 2009 Alexandre Quessy 
# http://alexandre.quessy.net
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# ToonLoop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ToonLoop.  If not, see <http://www.gnu.org/licenses/>.
"""
Libraries used by ToonLoop, the live stop motion tool.

osc_protocol.py is part of rawmaterials/Propulse[art] from SAT.
See https://svn.sat.qc.ca/trac/rawmaterials
"""

__version__ = '1.0 alpha'
import osc_protocol
__all__ = ("osc_protocol")

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

