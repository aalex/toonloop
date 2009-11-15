#!/usr/bin/env python
"""
File:    signal.py
Author:  Thiago Marcos P. Santos
Created: August 28, 2008

Purpose: A signal/slot implementation
Source: http://code.activestate.com/recipes/576477/

A Signal calls all the callbacks registered in its slots whenever it state 
changes. 
"""

from weakref import WeakValueDictionary

class Signal(object):
    """
    A Signal is callable. When called, it calls all the callables in its slots.
    """
    def __init__(self):
        self._slots = WeakValueDictionary()

    def __call__(self, *args, **kargs):
        for key in self._slots:
            func, _ = key
            func(self._slots[key], *args, **kargs)

    def connect(self, slot):
        """
        Slots must call this to register a callback method.
        :param slot: callable
        """
        key = (slot.im_func, id(slot.im_self))
        self._slots[key] = slot.im_self

    def disconnect(self, slot):
        """
        They can also unregister their callbacks here.
        :param slot: callable
        """
        key = (slot.im_func, id(slot.im_self))
        if key in self._slots:
            self._slots.pop(key)

    def clear(self):
        """
        Clears all slots
        """
        self._slots.clear()

if __name__ == "__main__":
    # Sample usage:
    class Model(object):
        def __init__(self, value):
            self._value = value
            self.changed = Signal()

        def set_value(self, value):
            self._value = value
            self.changed() # Emit signal

        def get_value(self):
            return self._value

    class View(object):
        def __init__(self, model):
            self.model = model
            model.changed.connect(self.model_changed)

        def model_changed(self):
            print("New value: %s" % (self.model.get_value()))

    model = Model(10)
    view1 = View(model)
    view2 = View(model)
    view3 = View(model)

    model.set_value(20)
    del view1 # remove one listener
    model.set_value(30)
    model.changed.clear() # remove all listeners
    model.set_value(40)

    # To disconnect : 
    # model.changed.disconnect(view1.model_changed)

