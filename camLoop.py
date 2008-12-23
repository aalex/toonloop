#!/usr/bin/python
## camLoop.py
##
## Copyright 2008 Tristan Matthews 
## <tristan@sat.qc.ca>
##
## Original idea by Alexandre Quessy
## http://alexandre.quessy.net
##
## camLoop.py is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## camLoop.py is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with camLoop.py.  If not, see <http://www.gnu.org/licenses/>.
##
## Camera module for pygame available from pygame's svn revision 1744 or greater
## svn co svn://seul.org/svn/pygame/trunk

import pygame, random
import pygame.camera
from pygame.locals import *

versionNumber = 0.1

pygame.init()

class VideoCapturePlayer(object):
    def __init__(self, **argd):
        self.imgWidth = 640
        self.width = self.imgWidth * 2
        self.height = 480 
        self.size = (self.width, self.height)
        self.__dict__.update(**argd)
        super(VideoCapturePlayer, self).__init__(**argd)
        self.surface = pygame.display.set_mode(self.size)
    
        pygame.display.set_caption("Loop Cam")
        self.camera = pygame.camera.Camera("/dev/video0", (self.imgWidth, self.height))
        self.camera.start()
        self.clock = pygame.time.Clock()
        self.frames = 0
        self.imageList = []
        self.imageIdx = 0
        
    def get_and_flip(self):
        self.lastImage = self.camera.get_image()
        self.surface.blit(self.lastImage, (0, 0))
        if len(self.imageList) > self.imageIdx:
            self.surface.blit(self.imageList[self.imageIdx], (self.imgWidth, 0))
            self.imageIdx += 1
        else:
            self.imageIdx = 0
        pygame.display.update()

    def grabImage(self):
        self.imageList.append(self.lastImage)

    def pause(self):
        self.paused = not self.paused

    def resetLoop(self):
        self.imageList = []
        self.resetPlaybackWindow()

    def popOneFrame(self):
        if self.imageList != []:
            self.imageList.pop()
            if self.imageList == []:
                self.resetPlaybackWindow()

    def resetPlaybackWindow(self):
        blankSurface = pygame.Surface((self.imgWidth, self.height))
        playbackPos = (self.imgWidth, 0)
        self.surface.blit(blankSurface, playbackPos)

    def printHelp(self):
        print "Usage: "
        print "<Space bar> = add image to loop "
        print "<Backspace> = remove image from loop "
        print "r = reset loop"
        print "p = pause"
        print "i = print current loop frame number, number of frames in loop and global framerate"
        print "h = print this help message"
        print "<Esc> or q = quit program\n"

    def printStats(self):
        print "Frame idx: " + str(self.imageIdx)
        print "Num images: " + str(len(self.imageList))
        print str(self.frames) + " fps\n"

    def main(self):
        print "CamLoop - Version " + str(versionNumber)
        print "---------------------------------------"
        print "Press h for usage instructions\n"
        running = True
        self.paused = False
        while running:
            events = pygame.event.get()
            for e in events:
                if e.type == QUIT:
                    running = False
                elif e.type == KEYDOWN: 
                    if (e.key == K_SPACE):
                        self.grabImage()
                    elif (e.key == K_r):
                        self.resetLoop()
                    elif (e.key == K_p):
                        self.pause()
                    elif (e.key == K_i): 
                        self.printStats()
                    elif (e.key == K_h):
                        self.printHelp()
                    elif (e.key == K_BACKSPACE):
                        self.popOneFrame()
                    elif (e.key == K_ESCAPE or e.key == K_q):
                        running = False

            if not self.paused:
                self.get_and_flip()
                self.clock.tick()
                self.frames = self.clock.get_fps()

VideoCapturePlayer().main()

