#!/usr/bin/env python
"""
Formalizing the effects.
"""
from toon import optgroup

class Effect(object):
    """
    Video Effect base class
    """
    def __init__(self):
        self.enabled = False
        self.loaded = False # set to False in case of error.
        self.options = optgroup.OptionsGroup() # to be replaced in children classes by a subclass of OptionsGroup

    def setup(self):
        pass

    def enable(self):
        self.enabled = True

    def disable(self):
        self.enabled = False

    def pre_draw(self):
        pass
    
    def post_draw(self):
        pass
