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
from rats import sig
import unittest

VERBOSE = False

class Test_Signal_Slot(unittest.TestCase):
    """
    Tests the signal/slot pattern.
    """
    def test_sig(self):
        class Model(object):
            def __init__(self, value):
                self._value = value
                self.changed = sig.Signal()

            def set_value(self, value):
                self._value = value
                self.changed() # Emit signal

            def get_value(self):
                return self._value

        class View(object):
            def __init__(self, model, test):
                self.model = model
                self.test = test
                model.changed.connect(self.model_changed)

            def model_changed(self):
                self.new_value = self.model.get_value()
                if VERBOSE:
                    print("New value : %d" % (self.new_value))

        # create a model and 3 listeners:
        model = Model(10)
        view1 = View(model, self)
        view2 = View(model, self)
        view3 = View(model, self)
        # trigger one signal:
        model.set_value(20)
        # delete one listener:
        del view1
        model.set_value(30)
        # To disconnect : 
        model.changed.disconnect(view2.model_changed)
        model.set_value(15)
        if view2.new_value == 15:
            self.fail("view2 slot should not have been triggered.")
        # remove all listeners:
        model.changed.clear() 
        model.set_value(40)
        if view3.new_value == 40:
            self.fail("view3 slot should not have been triggered.")
