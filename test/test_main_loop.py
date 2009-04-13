from twisted.trial import unittest
from twisted.internet import reactor

# import toonloop
def test():
    pass

class Test_1_Reactor(unittest.TestCase):
    def test_1_simplest(self):
        return reactor.callLater(1, test)

