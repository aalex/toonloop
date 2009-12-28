from toon import fx
"""
Effect that does nothing
"""
class NoEffect(fx.Effect):
    def __init__(self):
        fx.Effect.__init__(self)
        self.name = "None"

    def setup(self):
        self.loaded = True

def create_effect():
    return NoEffect()

