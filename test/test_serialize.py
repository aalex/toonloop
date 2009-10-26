"""
Unit test for rats/serialize.py
"""
from twisted.trial import unittest

from rats import serialize

# test classes with useless data
class Test(serialize.Serializable):
    def __init__(self):
        self.egg = 2
        self.spam = [3, 4, 5, 6]
        self.ham = 'werqwer'
class Cat(serialize.Serializable):
    def __init__(self):
        self.egg = 1
        self.spam = 2
class Rat(serialize.Serializable):
    def __init__(self):
        self.ham = 3
        self.cat = Cat()

filename = '/tmp/test_serialize.txt'
data = None

class Test_Serialize(unittest.TestCase):
    def setUp(self):
        global data
        global filename
        self.data = data
        self.filename = filename

    def test_load(self):
        raise unittest.SkipTest('This serialize module is deprecated')
        try:
            self.data = serialize.load(filename)
        except serialize.SerializeError:
            pass
        # print 'loaded objects', self.data

    def test_save(self):
        raise unittest.SkipTest('This serialize module is deprecated')
        self.data = [Test(), Rat()]
        serialize.save(filename, self.data)
        # print 'saved data'

