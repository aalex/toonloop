#!/usr/bin/env python
#
# ToonLoop Joystick
#
# Copyright 2009 Alexandre Quessy
# <alexandre@quessy.net>
#
# This file is part of ToonLoop.
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
import sys
import pygame.joystick

class JoystickManager:
    def __init__(self, **kwargs):
        self.joystick = None
        self.__dict__.update(kwargs)

if __name__ == '__main__':
    print 'start'
    pygame.joystick.init()
    print 'count ', pygame.joystick.get_count()
    try:
        id = 0
        joystick = pygame.joystick.Joystick(id)
    except:
        print sys.exc_info()
    else:
        print joystick.get_name(), joystick.get_id()
    print 'quit'
    pygame.joystick.quit()

