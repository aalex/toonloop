#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Purepy
# Copyright (C) 2009 Alexandre Quessy
# http://alexandre.quessy.net
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Purepy is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Purepy. If not, see <http://www.gnu.org/licenses/>.

import random 
from rats import fudi
from zope.interface import implements # TODO

class Obj(object):
    """
    Generic Pure Data Object.
    """
    # TODO implements IElement
    def __init__(self, name, *args, **keywords):
        self.name = name
        self.args = args
        #self.subpatch_name = "default"
        self.pos = [random.randrange(10, 600), random.randrange(10, 400)]
        if keywords.has_key("pos"):
            self.pos = keywords["pos"]

    def get_fudi(self):
        """ 
        Converts to list suitable for FUDI creation message. 
        """
        # self.subpatch_name, 
        li = ["obj", self.pos[0], self.pos[1], self.name]
        li.extend(self.args)
        return li

class Receive(Obj):
    """
    The [receive] Pure Data object.
    """
    # TODO implements IElement
    def __init__(self, receive_symbol):
        self.receive_symbol = receive_symbol
        Obj.__init__(self, "r", receive_symbol)

    def send(self, *args):
        """
        Returns a FUDI message suitable to be sent to that [receive] object. 
        """
        li = [self.receive_symbol]
        li.extend(args)
        return li

class Connection(object):
    """
    Connection between two Pure Data objects.
    """
    # TODO implements IElement
    def __init__(self, from_object, from_outlet, to_object, to_inlet):
        self.from_object = from_object
        self.from_outlet = from_outlet
        self.to_object = to_object
        self.to_inlet = to_inlet
        # self.subpatch_name = "default"

    def get_fudi(self):
        """
        Returns fudi creation list.
        """
        # self.subpatch_name, 
        return ["connect", self.from_object.id, self.from_outlet, self.to_object.id, self.to_inlet]

class SubPatch(object):
    """
    Pure Data Subpatch. 
    """
    def __init__(self, name=None):
        self.name = name
        self.objects = []
        self.connections = []
    
    def get_fudi(self):
        """
        Return FUDI lists for the whole subpatch.
        Objects and connections
        """
        li = []
        print "objects"
        for obj in self.objects:
            if self.name is None:
                l = []
            else:
                l = [self.name]
            l.extend(obj.get_fudi())
            li.append(l)
            print l
        print "connections"
        for conn in self.connections:
            if self.name is None:
                l = []
            else:
                l = [self.name]
            l.extend(conn.get_fudi())
            li.append(l)
            print l
        print "done creating FUDI list"
        return li

    def obj(self, name, *args, **keywords):
        """
        Adds an object to the supatch.
        Factory that wraps the Obj constructor.
        @return Obj instance.
        """
        obj = Obj(name, *args, **keywords)
        return self._add_object(obj)

    def _add_object(self, obj):
        """ Common to obj() and receive(). """
        obj.id = len(self.objects)
        # obj.subpatch_name = self.name
        self.objects.append(obj)
        return obj
        
    def receive(self, receive_symbol):
        """
        Similar to obj(), but for a receive object only.
        Appends a [receive] object to the subpatch.
        """
        obj = Receive(receive_symbol)
        return self._add_object(obj)
    
    def connect(self, from_object, from_outlet, to_object, to_inlet):
        """
        Connects two objects together.
        Returns None
        """
        if from_object not in self.objects:
            raise PureError("%s object not in subpatch %s" % (from_object, self))
        elif to_object not in self.objects:
            raise PureError("%s object not in subpatch %s" % (to_object, self))
        else:
            conn = Connection(from_object, from_outlet, to_object, to_inlet)
            self.connections.append(conn)
            # conn.subpatch_name = self.name
            
    def clear(self):
        """
        Returns a message to clear the subpatch.
        """
        # TODO: send it directly
        self.connections = []
        self.objects = []
        return ["pd-%s" % (self.name), "clear"]

if __name__ == "__main__":
    sub = SubPatch("hello_01")
    tgl = sub.obj("tgl")
    metro = sub.obj("metro", 50)
    sub.connect(tgl, 0, metro, 0)
    li = sub.get_fudi()
    print "result:"
    print li
    for i in li:
        if len(i) == 0:
            print(fudi.to_fudi(i[0]))
        else:
            print(fudi.to_fudi(i[0], *i[1:]))




