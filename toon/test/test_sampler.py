#!/usr/bin/env python
import unittest
from toon import sampler

class Test_Allocator(unittest.TestCase):
    def test_add_remove(self):
        allocator = sampler.Allocator(5)
        NUM = 5
        for i in range(NUM):
            value = allocator.allocate()
            assert(value == i)
        try:
            allocator.allocate()
        except sampler.AllocationError, e:
            pass
        else:
            self.fail("Trying to allocate should have thrown an error.")
        for i in range(NUM):
            allocator.free(i)
        try:
            allocator.free(0)
        except sampler.AllocationError, e:
            pass
        else:
            self.fail("Trying to free an already freed value should have thrown an error.")
        
