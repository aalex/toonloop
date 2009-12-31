#!/usr/bin/env python
"""
Formalizing the effects.
"""
from toon import optgroup
from twisted.python import modules

class Effect(object):
    """
    Video Effect base class
    """
    def __init__(self):
        self.name = None # Must be set to a str in children classes.
        self.enabled = True
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


def load_effects():
    """
    Loads modules in the toon.effects package and returns a dict with 
    their name as the key
    """
    ret = {}
    fx_package = modules.getModule("toon.effects")
    for module in fx_package.iterModules():
        try:
            loaded_module = module.load()
            effect = loaded_module.create_effect()
        except Exception, e:
            print("Error loading %s effect module : %s " % (module.name, e.message))
            #raise
        else:
            # the create_effect may return None, to disable the effect.
            if effect is not None:
                effect.setup()
                if not effect.loaded:
                    print("Could not load effect %s." % (effect.name))
                else:
                    ret[effect.name] = effect
    return ret
