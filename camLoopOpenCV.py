#!/usr/bin/env python
# camLoopOpenCV.py
#
# Copyright 2008 Tristan Matthews 
# <le.businessman@gmail.com>
#
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# camLoopOpenCV.py is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# camLoopOpenCV.py is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with camLoop.py.  If not, see <http://www.gnu.org/licenses/>.
#
# Requires CVS checkout of opencv. Installation details described here:
# http://tristanswork.blogspot.com/2008/12/computer-vision-on-os-x-with-python-and.html
# FIXME: Weird event handling issue where it blocks until it gets an 
# initial keyboard or mouse event. 
# FIXME: Opens in two windows, should be all in one window. However this means
# copying both images to a new bigger image and only displaying that. 
# See http://opencv.willowgarage.com/wiki/DisplayManyImages for an example of
# how to do this.

from opencv.cv import *
from opencv.highgui import *
    
versionNumber = 0.1

class CamLoop():
    def __init__(self):
        self.imgWidth = 640
        self.width = self.imgWidth
        self.height = 480
        self.size = (self.width, self.height)
        cvNamedWindow("camLoops", 1)
        cvNamedWindow("Playback", 1)
        self.imageList = []
        self.imageIdx = 0
        self.capture = cvCreateCameraCapture(0)
        cvSetCaptureProperty(self.capture, CV_CAP_PROP_FPS, 30.0)
        # blank image with bitdepth 8 and 3 channels
        self.blankImage = cvCreateImage(cvSize(self.width, self.height), 8, 3)
        self.blackoutLoop()

    def blackoutLoop(self):
        cvShowImage("camLoops", self.blankImage)

    def display(self):
        if len(self.imageList) > 0:
            if self.imageIdx < len(self.imageList) - 1:
                self.imageIdx += 1
            else:
                self.imageIdx = 0
            cvShowImage("camLoops", self.imageList[self.imageIdx])
            #print self.imageIdx
        
        cvShowImage("Playback", self.lastImage)

    def grabImage(self):
        self.imageList.append(cvCloneImage(self.lastImage))
    
    def pause(self):
        self.paused = not self.paused

    def resetLoop(self):
        self.imageList = []
        self.blackoutLoop()

    def printHelp(self):
        print "Usage: "
        print "<Space bar> = add image to loop "
        print "r = reset loop"
        print "p = pause"
        print "i = print current loop frame number and number of frames in loop"
        print "h = print this help message"
        print "<Esc> or q = quit program\n"

    def printStats(self):
        print "Frame idx: " + str(self.imageIdx)
        print "Num images: " + str(len(self.imageList))

    def cleanup(self):
        """ Frees previously allocated resources """
        cvDestroyWindow("camLoops")
        cvDestroyWindow("Playback")
        cvReleaseImage(self.blankImage)
        for img in self.imageList:
            cvReleaseImage(img)
        cvReleaseCapture(self.capture)

    def main(self):
        print "CamLoop - Version " + str(versionNumber)
        print "---------------------------------------"
        print "Press h for usage instructions\n"
        running = True
        self.paused = False
        if self.capture:
            running = True
            while running: 
                key = cvWaitKey(20)
                self.lastImage = cvQueryFrame(self.capture)
                if not self.lastImage:
                    running = False
                if key == ' ':
                    self.grabImage()
                elif key == 'r':
                    self.resetLoop()
                elif key == 'p':
                    self.pause()
                elif key == 'i':
                    self.printStats()
                elif key == 'h':
                    self.printHelp()
                elif key == 'q':
                    running = False

                if not self.paused:
                    self.display()
            
        self.cleanup()

CamLoop().main()

