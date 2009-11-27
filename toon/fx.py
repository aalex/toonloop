#!/usr/bin/env python
"""
Formalizing the effects.
"""
class Effect(object):
    """
    Video Effect base class
    """
    def __init__(self):
        self.enabled = False

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
