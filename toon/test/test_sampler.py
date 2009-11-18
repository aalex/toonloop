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

class Test_Mapper(unittest.TestCase):
    def _on_record(self, buffer_id):
        self._called_record = True

    def _on_clear(self, buffer_id):
        self._called_clear = True

    def test_add_remove(self):
        self._called_record = False
        self._called_clear = False
        mapper = sampler.Mapper(num_clips=2, num_sounds=10)
        ret = mapper.add(0, 0)
        if not type(ret) is int:
            self.fail("Add should have returned a number.")
        ret = mapper.remove(0, 0)
        if not type(ret) is int:
            self.fail("Clear should have returned a number.")
        for frame_id in range(10):
            mapper.add(0, frame_id)
        try:
            mapper.add(1, 0)
        except sampler.AllocationError, e:
            pass
        else:
            self.fail("Should have had no buffer left to allocate.")
        for frame_id in range(10):
            mapper.remove(0, frame_id)
        
