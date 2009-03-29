"""
Motion tween equations.
"""
#  Easing Equations v1.5
#  May 1, 2003
#  (c) 2003 Robert Penner, all rights reserved. 
#  This work is subject to the terms in 
#  http://www.robertpenner.com/easing_terms_of_use.html.  
#  
#  These tweening functions provide different flavors of 
#  math-based motion under a consistent API. 
#  
#  Types of easing:
#  
#	  Linear
#	  Quadratic
#	  Cubic
#	  Quartic
#	  Quintic
#	  Sinusoidal
#	  Exponential
#	  Circular
#	  Elastic
#	  Back
#	  Bounce
#
#  Changes:
#  1.5 - added bounce easing
#  1.4 - added elastic and back easing
#  1.3 - tweaked the exponential easing functions to make endpoints exact
#  1.2 - inline optimizations (changing t and multiplying in one step)
#        -- thanks to Tatsuo Kato for the idea
#  
#  Discussed in Chapter 7 of 
#  Robert Penner's Programming Macromedia Flash MX
#  (including graphs of the easing equations)
#  
#  http://www.robertpenner.com/profmx
#  http://www.amazon.com/exec/obidos/ASIN/0072223561/robertpennerc-20
#

LINEAR_TWEEN = 1

class Tween(object):
    """
    Motion tween tool.
    
    """
    def __init__(self, start_val, end_val, duration, current_time, 
            kind=LINEAR_TWEEN):
        # TODO: use time from pygame of os ?
        # self.t = current_time # current time
        self.b = start_val # beginning value
        self.c = end_val - start_val # change in value
        self.d = duration # duration
        self.kind = kind
    
    def tick(self, current_time):
        """
        Simple wrapper for easing methods.
        
        All the easing methods take those arguments :
        t: current time
        b: beginning value
        c: change in value
        d: duration
        """
        method = self.linear_tween
        if self.kind == LINEAR_TWEEN:
            method = self.linear_tween
        return method(float(current_time), self.b, self.c, self.d)

    def linear_tween(self, t, b, c, d):
        """
        simple linear tweening - no easing
        """
        return c * (t / d) + b

if __name__ == '__main__':
    print "this test will tween from 0 to 100 in steps of 10"
    kind = LINEAR_TWEEN
    start_val = 0.0
    end_val = 100.0
    duration = 10.0
    current_time = 0.0
    tween = Tween(start_val, end_val, duration, current_time, kind)
    for t in range(11):
        print tween.tick(t)

