#!/usr/bin/env python
#
# Toonloop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# <alexandre@quessy.net> & <le.businessman@gmail.com>
# http://www.toonloop.com
#
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# Toonloop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Toonloop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
#
from rats.serialize import Serializable

class ToonSequence(Serializable):
    """
    Not used yet !!!!!!
    
    Toonloop sequence. 

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

def load_image(file_name, colorkey=None):
    """
    Loads an image file.
    returns surface, width, height
    """
    try:
        image = pygame.image.load(file_name)
    except pygame.error, message:
        print 'Cannot load image:', file_name
    image = image.convert()

    if colorkey is not None:
        if colorkey is -1:
            colorkey = image.get_at((0,0))
            image.set_colorkey(colorkey, RLEACCEL)
    return image, image.get_rect()

# class ToonClip :

        # to do:
        #self.writehead = 0
        #self.images_file_names = []
        #self._intervalometer_delayed_id = None
        #self.intervalometer_enabled = False
        #self.intervalometer_rate_seconds = 0.1

# class ToonProject(object):
#     """
#     Project file with clips and sequences.
# 
#     Serializes project and save image files.
#     """
#     def __init__(self, name, **kwargs):
#         self.path = os.path.expanduser("~/toonloop")
#         self.name = "toonloop"
#         self.datetime = self.now()
#         self.__dict__.update(argd)
#         # self.clips = []
# 
#     def now(self):
#         return strftime("%Y-%m-%d_%Hh%Mm%S")
#         
#     def __str__(self):
#         """
#         Full path of the project folder.
#         """
#         return "%s/%s_%s" % (self.path, self.name, self.datetime)
