#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# RawMaterials
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# RawMaterials is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with RawMaterials.  If not, see <http://www.gnu.org/licenses/>.

"""
State Saving and Configuration tools

This code is not even alpha.
"""

# default_file_name = 'TODOchangeme.txt'
# import copy


class Serializable(object):
    """
    Serializable = whose state can be saved and retrieved.
    Every class with state saving should extend this class.
    Use with StateSaver
    """

    def __init__(self, attributes={}):
        """
        Children classes should override this to provide 
        default values for each attribute.
        """
        self._init_attributes(attributes)
    
    def _init_attributes(self, attributes):
        """
        Must be called at startup with 1st arg.
        Call this after having set the default attributes
        in your __init__ method.
        """
        self.__dict__.update(attributes)
        
    def serialize(self):
        """
        Returns any type representing how we save the sate of this instance.
        Can be list, tuple, dict or even string or int.
        
        This method could be overridden in children classes.
        
        Might need to use copy.deepcopy().
        """
        # used to return None
        recursive = True
        ret = {} #dict
        ret['_objects'] = {} # dict name:(class, attributes)
        # copy.copy(self.__dict__)
        # shallow copy of a dict does duplicate objects. 
        for key, value in self.__dict__.items():
            if type(value) is object:
                try:
                    value = value.serialize()
                    klass = self.__class__.__name__
                    ret['_objects'][key] = (klass, value)
                except NameError:
                    pass
            else:
                value = value # no conversion to str here
                ret[key] = value
        return ret

    def unserialize(self, data):
        """
        Retrieve former state 
        Arg is data as returned by serialize()
        This method must be overridden in children classes.
        """
        # pass
        recursive = True
        for key, value in data.items():
            if key == '_objects':
                for attr_name, klass_value in value.items():
                    klass, value = klass_value
                ret[attr_name] = klass(value)
            if type(value) is object:
                try:
                    value = value.serialize()
                    ret['_objects'][key] = value
                except NameError:
                    pass
            else:
                value = value # no conversion to str here
                ret[key] = value
        return ret

# import shelve

class StateSaver(object):
    """
    State saving utility. 
    
    State saving utility. Uses classes that extend Serializable.
    """
    def __init__(self, filename=None):
        self.dico = {}
        self.filename = filename
        # self.dico = shelve.open(filename, protocol=2)
        if filename is not None:
            self.load(filename)
    
    
    def get(self, key):
        """
        Gets a key's value.

        Returns None if not set
        """
        try:
            return self.dico[key]
        except KeyError, e:
            # raise
            return None

    def set(self, key, val):
        """
        val must be a Serializable subclass.

        Use mutable types such as objects so that 
        it values keep in sync, since same reference.
        """
        self.dico[key] = val

    def serialize(self):
        """
        Turns current dict into a readable string to be saved.
        """
        ret = {}
        for key, val in self.dico.items():
            ret[key] = val.serialize()
        return repr(ret)
    
    def unserialize(self, data):
        """
        Text to dict conversion.
        """
        tmp_dict = eval(data, {}, {})
        ret = {}
        for key, val in tmp_dict.items():
            ret[key] = val.unserialize()
        return ret

    def save(self, filename=None):
        """
        Writes to file
        """
        # self.dico.sync()
        if filename is None and self.filename is None:
            print "you must specify a file name"
        else:
            if filename is None:
                filename = self.filename
            else:
                self.filename = filename
            try:
                f = open(filename, "w")
                txt = self.serialize()
                f.write(txt)
                f.close()
            except OSError, e:
                print "error in save: ", e
            except IOError, e:
                print e
    
    def load(self, filename):
        """
        Reads from a file.
        """
        data = {}
        try:
            f = open(filename, "r")
            txt = f.read()
            f.close()
        except IOError, e:
            print e
        except OSError, e:
            print e
        else:
            print "Loading dict from", filename

            data = repr(txt)
        self.dico = data

    def __del__(self):
        # self.dico.close()
        pass

if __name__ == '__main__':
    # test classes with useless data
    class Cat(Serializable):
        def __init__(self, data={}):
            self.egg = 1
            self.spam = 2
            self._init_attributes(data)

    class Rat(Serializable):
        def __init__(self, data={}):
            self.ham = 3
            self.cat = Cat()
            self._init_attributes(data)

    # d = RawStateSaver('test.state.saver.dat')
    # foo = d.get('foo')
    # print 'foo:',foo
    # if foo is None:
    #     foo = 'qweqweqwe'
    # d.set('foo',foo)
    # d.save()
    # test -------------
    
    filename = 'test_data.txt'

    state = StateSaver(filename)
    rat = state.get('rat')
    # b = d.get('b')
    #print "initial b = ", b
    print "initial rat:", rat
    # d.set('b', 3.14159)
    state.set('rat', Rat())

    print "saving"
    print state.save(filename)


