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
"""
Saves the images and movie file from a clip.
"""
import sys
from time import strftime
import os
import shutil
import glob
import pprint

from twisted.internet import reactor
from twisted.internet import defer
#from twisted.python import failure

from toon import mencoder
import pygame
from pygame.locals import *

class ClipSaverError(Exception):
    """
    Any error that may occur during saving a clip.
    """
    pass

class ClipSaver(object):
    """
    Saves a clip to images and a movie file if possible.
    """
    def __init__(self, core, dir_path, file_prefix, clip_id):
        self.clip_id = clip_id
        self.core = core # toonloop app
        self.file_prefix = file_prefix
        self.dir_path = dir_path
        self.current_index = 0 # saving which image next
        self.IMAGES_DIR = "images"
        self._deferred = None
        self.is_busy = False

    def save(self):
        """
        Does save the clip.
        Return a deferred.
        """
        self.is_busy = True
        if self._deferred is not None:
            raise ClipSaverError("Do not call save twice on the same ClipSaver.")
        self._deferred = defer.Deferred()
        try:
            if not os.path.exists(self.dir_path):
                os.makedirs(self.dir_path)
                print('mkdir %s' % (self.dir_path))
        except OSError, e:
            msg = 'Error creating directories' % (self.dir_path, e.message)
            self._fail(msg)
            print(msg)
        else:
            try:
                data_subdir = os.path.join(self.dir_path, self.IMAGES_DIR)
                if not os.path.exists(data_subdir):
                    os.makedirs(data_subdir)
                    print('mkdir %s' % (data_subdir))
            except OSError, e:
                msg = 'Error creating directories' % (data_subdir, e.message)
                print(msg)
                self._fail(msg)
            else:
                reactor.callLater(0.0, self._write_01_next_image)
        return self._deferred

    def _fail(self, msg):
        self.is_busy = False
        self._deferred.errback(msg)

    def _succeed(self, msg):
        self.is_busy = False
        self._deferred.callback(msg)

    def _write_01_next_image(self):
        """
        Saves each image using twisted in order not to freeze the app.
        Uses the JPG extension.
        """
        # TODO : use clip_id
        if self.current_index < len(self.core.clips[self.clip_id].images):
            name = ("%s/%s_%5d.jpg" % (self.dir_path, self.file_prefix, self.current_index)).replace(' ', '0')
            if self.core.config.verbose:
                print("writing image %s" % (self.current_index))
            pygame.image.save(self.core.clips[self.clip_id].images[self.current_index], name) # filename extension makes it a JPEG
            self.current_index += 1
            reactor.callLater(0.0, self._write_01_next_image)
        else:
            reactor.callLater(0.0, self._write_02_images_done)
    
    def _write_02_images_done(self):
        """
        Converts the list of images in a motion-JPEG .mov video file.
        """
        if self.current_index > 0:
            if self.core.config.verbose:
                print("\nConverting to motion JPEG in Quicktime container.")
            fps = 6 # TODO
            #self.clip.increment_every # self.clip.framerate
            #fps = self.renderer.desired_fps 

            deferred = mencoder.jpeg_to_movie(self.file_prefix, self.dir_path, fps, self.core.config.verbose, self.core.config.image_width, self.core.config.image_height)
            deferred.addCallback(self._write_03_movie_done)
            deferred.addErrback(self._eb_mencoder)
            # to do : serialize clips with file names
            # self.project_file = 'project.txt'

    def _eb_mencoder(self, reason):
        msg = reason.getErrorMessage()
        print(msg)
        self._fail(msg)

    def _write_03_movie_done(self, results): # deferred callback
        """
        Called when mencoder conversion is done.
        MOV file.
        """
        if self.core.config.verbose:
            print("Done converting %s/%s.mov" % (self.dir_path, self.file_prefix))
        reactor.callLater(1.0, self._write_04_delete_images)

    def _write_04_delete_images(self):
        """
        deletes JPG images or moves them to the 'data' folder in the project folder.
        renames MOV file.
        """
        files = glob.glob("%s/%s_*.jpg" % (self.dir_path, self.file_prefix))
        for f in files:
            if self.core.config.delete_jpeg: # delete images
                try:
                    os.remove(f)
                    if self.core.config.verbose:
                        print('removed %s' % (f))
                except OSError, e:
                    msg = "%s Error removing file %s" % (e.message, f)
                    print(msg)
                    #self._fail(msg)
            else: # move images
                try:
                    dest = os.path.join(self.dir_path, self.IMAGES_DIR, os.path.basename(f))
                    shutil.move(f, dest)
                    #if self.core.config.verbose:
                    #    print('moved %s to %s' % (f, dest))
                except IOError, e:
                    msg = "%s Error moving file %s to %s" % (e.message, f, dest)
                    print(msg)
                    #self._fail(msg)
        # rename final movie file
        try:
            src = "%s/%s.mov" % (self.dir_path, self.file_prefix)
            dest = "%s/clip_%s.mov" % (self.dir_path, self.file_prefix)
            shutil.move(src, dest)
            if self.core.config.verbose:
                print('renamed %s to %s' % (src, dest) )
                print('DONE SAVING CLIP %s' % (self.clip_id))
        except IOError, e:
            msg = "%s Error moving file %s to %s" % (e.message, src, dest)
            print(msg)
            self._fail(msg)
        else:
            self._succeed("Successfully saved images, converted movie and moved files for clip %s" % (self.clip_id))

