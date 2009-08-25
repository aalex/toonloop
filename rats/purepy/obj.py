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

class Obj(object):
    """
    Generic Pure Data Object.
    """
    def __init__(self, name, *args, **keywords):
        self.name = name
        self.args = args
        self.subpatch_name = "default"
        self.pos = [random.randrange(10, 600), random.randrange(10, 400)]
        if keywords.has_key("pos"):
            self.pos = keywords["pos"]

    def get_fudi(self):
        """ 
        Converts to list suitable for FUDI creation message. 
        """
        li = [self.subpatch_name, "obj", self.pos[0], self.pos[1], self.name]
        li.extend(self.args)
        return li

class Receive(Obj):
    """
    The [receive] Pure Data object.
    """
    def __init__(self, receive_symbol):
        Obj.__init__(self, "r", receive_symbol)

    def send(self, *args):
        """
        Returns a FUDI message suitable to be sent to that [receive] object. 
        """
        li = [self.args[0]]
        li.extend(args)
        return li

class Connection(object):
    """
    Connection between two Pure Data objects.
    """
    def __init__(self, from_object, from_outlet, to_object, to_inlet):
        self.from_object = from_object
        self.from_outlet = from_outlet
        self.to_object = to_object
        self.to_inlet = to_inlet
        self.subpatch_name = "default"

    def get_fudi(self):
        """
        Returns fudi creation list.
        """
        return [self.subpatch_name, "connect", self.from_object.id, self.from_outlet, self.to_object.id, self.to_inlet]

class SubPatch(object):
    """
    Pure Data Subpatch. 
    """
    def __init__(self, name):
        self.name = name
        self.objects = []
        self.connections = []
    
    def get_fudi(self):
        """
        Return FUDI lists for the whole subpatch.
        Objects and connections
        """
        li = []
        for obj in self.objects:
            li.append(obj.get_fudi())
        for conn in self.connections:
            li.append(conn.get_fudi())
        return li

    def obj(self, name, *args, **keywords):
        """
        Adds an object to the supatch.
        Factory that wraps the Obj constructor.
        @return Obj instance.
        """
        obj = Obj(name, *args, **keywords)
        return self._add(obj)

    def _add(self, obj):
        """ Common to obj() and receive(). """
        obj.id = len(self.objects)
        obj.subpatch_name = self.name
        self.objects.append(obj)
        return obj
        
    def receive(self, receive_symbol):
        """
        Similar to obj(), but for a receive object only.
        Appends a [receive] object to the subpatch.
        """
        obj = Receive(receive_symbol)
        return self._add(obj)
    
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
            conn.subpatch_name = self.name
            
    def clear(self):
        """
        Returns a message to clear the subpatch.
        """
        # TODO: send it directly
        self.connections = []
        self.objects = []
        return ["pd-%s" % (self.name), "clear"]

if __name__ == "__main__":
    s = SubPatch("hello_01")
    tgl = s.obj("tgl")
    metro = s.obj("metro", 50)
    s.connect(tgl, 0, metro, 0)
    li = s.get_fudi()
    for i in li:
        print(fudi.to_fudi(*i))




