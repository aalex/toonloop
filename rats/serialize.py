#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Toonloop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# http://toonloop.com
# 
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# Toonloop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Toonloop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
#
"""
State saving utilies.

Easily readable and editable.
Support objects as well as simpler data types.

The original file is : toonloop/trunk/py/rats/serialize.py
"""
from twisted.spread import jelly
import pprint

class SerializeError(Exception):
    """
    Error occuring while trying to save serialized data.
    """
    pass

class UnserializeError(Exception):
    """
    Error occuring while trying to load serialized data.
    """
    pass

class Serializable(jelly.Jellyable):
    """
    Any class that is serializable using these tools
    should extend this one.
    """
    pass

def save(filename, obj):
    """
    Saves any python data type to a file.

    Might throw an SerializeError
    """
    global _verbose
    li = jelly.jelly(obj)
    try:
        f = open(filename, 'w')
        f.write(pprint.pformat(li))
        f.close()
    except IOError, e:
        raise SerializeError(e.message)
    except OSError, e:
        raise SerializeError(e.message)

def load(filename):
    """
    Loads any python data type from a file.

    Might throw an UnserializeError
    """
    try:
        f = open(filename, 'r')
        li = eval(f.read()) # do this only on trusted data !
        f.close()
    except IOError, e:
        raise UnserializeError(e.message)
    except OSError, e:
        raise UnserializeError(e.message)
    try:
        obj = jelly.unjelly(li)
    except jelly.InsecureJelly, e:
        raise UnserializeError(e.message)
    except AttributeError, e:
        raise UnserializeError(e.message)
    else:
        return obj

