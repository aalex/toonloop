#!/usr/bin/env python
"""
State saving tools

Two json modules exist. We must use the simplejson that became 
the standard json module in Python 2.6
"""
from rats import statesaving
import unittest
import tempfile

file_name = tempfile.mktemp()

class Test_State_Saving(unittest.TestCase):
    def test_01_save(self):
        global file_name
        data = {}
        data['x'] = 2
        data['y'] = 3.14159
        data['z'] = [1, 2, 3, 4, 5, 6]
        data['a'] = {'egg':'white', 'ham':'pink'}
        statesaving.save(file_name, data)

    def test_02_load(self):
        global file_name
        data = statesaving.load(file_name)
        assert(data['x'] == 2)
        assert(data['y'] == 3.14159)
        assert(data['z'] == [1, 2, 3, 4, 5, 6])
        assert(data['a'] == {'egg':'white', 'ham':'pink'})
