# -*- coding: utf-8 -*-

# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.

"""
Subject and Observer classes to implement the Model-View-Controller pattern.
"""

import weakref

class Observer(object):
    """Instances of Observer are notified of changes happening in the
    Subject instances they are attached to.
    It's possible for an Observer to watch many Subject.
    """
    
    def __init__(self, subjects):
        self.subjects = []
        if isinstance(subjects, tuple):
            for subject in subjects:
                self.append(subject)
        else:
            self.append(subjects)
        
    def append(self, subject):
        """
        Adds a subject to be observed by this Observer instance.
        """
        if isinstance(subject, Subject):
            self.subjects.append(subject)        
            subject._attach(self)   # should we make an excepption/error message
                                    # if subject isn't a Subject instance ?
        
    def update(self, origin, key, value):
        """Called when an attribute of the observed object is changed.
        Should be overridden.

        @param origin:    observed object for which the attribute is changed (caller)
        @type origin:     any
        @param key:       attribute changed (default to the function that called it. in api.py)
        @type key:        string
        @param value:     value of the attribute after being set
        @type value:      any
        """
        raise NotImplementedError
       
       
class Subject(object):
    """
    Subject watched by an Observer. 
    
    This can be the "Model" in the Model-View-Controller pattern.
    """
    def __init__(self):
        self.observers = weakref.WeakValueDictionary()

    def _attach(self, observer):
        ob_id = id(observer)
        if ob_id not in self.observers:
            self.observers[ob_id] = observer
                    
    def notify(self, caller, value, key=None):
        """
        Calls all its observers that an attribute has changed.

        Usage: self.notify(self,'brown','color')
        WARNING: the signature has changed since miville.
        args key and valud have been interchanged.
        """
        if not key:
            raise Exception("Warning: support for None keys has been removed")
        #     key = common.get_def_name()
        for observer in self.observers.itervalues():
            observer.update(caller, key, value)
            
            
