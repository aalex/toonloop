#!/usr/bin/env python
#
# ToonLoop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# <alexandre@quessy.net> & <le.businessman@gmail.com>
# http://www.toonloop.com
#
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# ToonLoop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ToonLoop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with ToonLoop.  If not, see <http://www.gnu.org/licenses/>.
#
from rats.serialize import Serializable
from rats.observer import Subject

class ToonSequence(Serializable):
    """
    Not used yet !!!!!!
    
    ToonLoop sequence. 

    A sequence is made of many shots.
    
    An act include one or more sequences; sequences comprise one or more scenes; 
    and scenes may be thought of as being built out of shots (if one is thinking visually) 
    or beats (if one is thinking in narrative terms).
    """
    # TODO: use it !
    def __init__(self, **argd):
        self.shots_ids = []
        # end of overridable attributes
        self.__dict__.update(argd)

class ToonProject(object):
    """
    Project file with shots and sequences.
    """
    def __init__(self):
        self.name = "toonloop"

        
