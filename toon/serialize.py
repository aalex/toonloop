#!/usr/bin/env python
# -*- coding: utf-8 -*-
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
State saving utilies.

Easily readable and editable.
Support objects as well as simpler data types.
"""
from twisted.spread import jelly
import pprint

_verbose = True

class SerializeError(Exception):
    """
    Occur occuring while trying to save or load serialized data.
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

    Might throw an IOError
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
    if _verbose:
        print 'saved', li

def load(filename):
    """
    Loads any python data type from a file.

    Might throw an IOError
    """
    try:
        f = open(filename, 'r')
        li = eval(f.read())
        f.close()
    except IOError, e:
        raise SerializeError(e.message)
    except OSError, e:
        raise SerializeError(e.message)
    obj = jelly.unjelly(li)
    if _verbose:
        print 'loaded', li
    return obj 

if __name__ == '__main__':
    class Test(Serializable):
        def __init__(self):
            self.egg = 2
            self.spam = [3,4,5,6]
            self.ham = 'werqwer'
    # test classes with useless data
    class Cat(Serializable):
        def __init__(self):
            self.egg = 1
            self.spam = 2

    class Rat(Serializable):
        def __init__(self):
            self.ham = 3
            self.cat = Cat()


    filename = 'test_serialize.txt'
    try:
        s = load(filename)
        print 'loaded objects', s
    except SerializeError, e:
        print 'error: could not load file', e
    s = [Test(), Rat()]
    save(filename, s)
    print 'saved data'

