#!/usr/bin/env python
from toon import optgroup
from toon import fx
import unittest

class ExampleGroup(optgroup.OptionsGroup):
    def __init__(self):
        self.i = 3
        self.f = 2.0
        self.s = "qwe"
        self.vec4 = [1.0, 0.8, 0.2, 1.0]
        self.vec3 = [1.0, 0.8, 0.2]

class Test_Options(unittest.TestCase):
    def test_string_options(self):
        g = ExampleGroup()
        g.set_value("i", "4")
        if g.i != 4:
            self.fail("The attribute i should have been set to a int with value 4.")
        g.set_value("f", "4.2")
        if g.f != 4.2:
            self.fail("The attribute f should have been set to a float with value 4.2.")
        g.set_value("s", "asd")
        if g.s != "asd":
            self.fail("The attribute s should have been set to a str with value asd.")
        g.set_value("vec3", "5,6,7.2")
        if g.vec3[0] != 5.0:
            self.fail("vec3[0] has a wrong value %s" % (g.vec3[0]))
        if g.vec3[1] != 6.0:
            self.fail("vec3[1] has a wrong value %s" % (g.vec3[1]))
        if g.vec3[2] != 7.2:
            self.fail("vec3[2] has a wrong value %s" % (g.vec3[2]))

