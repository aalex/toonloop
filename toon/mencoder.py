#!/usr/bin/env python
#
# Toonloop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# http://toonloop.com
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
"""
Converts a series a JPEG images to a MJPEG-4 video file using mencoder.

The output will be in the same dirctory than the inputs. 
It is possible to append the path to the mencoder executable like this :
os.environ['PATH'] += ":/Applications/Toonloop.app/Contents/Resources/bin"

An other useful beautiful line of code : mencoder -oac copy -ovc copy *.mov -o final_movie.mov
"""

import os
import pprint
import sys
import traceback
try:
    import cStringIO as StringIO
except ImportError:
    import StringIO
from twisted.internet import defer
from twisted.internet import reactor
from twisted.internet import protocol
from twisted.internet import utils
from twisted.python import failure
from twisted.python import procutils
# TODO: import twisted log

# variables
mencoder_executable = ''
try:
    mencoder_executable = procutils.which('mencoder')[0] # gets the executable
except IndexError:
    print "mencoder must be in your $PATH"
    print "mencoder conversion is disabled"
    # sys.exit(1)
movie_name_suffix = ""

class MencoderError(Exception):
    """
    Any error this module can raise.
    """
    pass

def cb_error(command_failure, args):
    print "FAILURE"
    pprint.pprint({'failure':command_failure, 'exception':sys.exc_info(), 'args':args})
    # traceback.print_tb(sys.exc_info()[2])
    print '>>>> Exception message : %s' % (command_failure.value.message)
    
def cb_success(result, args):
    """
    Called once a bunch of child processes are done.
    
    @param success_results a tuple of (boolean,str) tuples
    @param commands is a list of the provided commands
    @param callback is a callback to call once done.
    
    Args are: the command that it the results if from, text data resulting from it, callback to call once done.
    callback should be called with the same arguments. command, results, callback (but results can be something else than text)
    
    This is the default callback for commands. 
    You should provide an other callback as argument to the commands starter functions.
    You can copy-paste this one to start with.
    """
    print "SUCCESSFULLY converted JPEG images to movie"
    print "mencoder arguments:", args
    print "\nmencoder output:"
    print result[0] # stdout 

#     success, results_infos = result
#     command = commands[i]
#     print ">>>>>>>>>>>>>>>>  command : %s   <<<<<<<<<<<<<<<" % (command)
#     if isinstance(results_infos, failure.Failure):
#         print ">>>> FAILURE : ", results_infos.getErrorMessage()  # if there is an error, the programmer should fix it.
#     else:
#         stdout, stderr, signal_or_code = results_infos
#         print ">>>> stdout: %s" % (stdout)
#         print ">>>> stderr: %s" % (stderr)
#         if success:
#             print ">>>> Success !"
#             print ">>>> code is ", signal_or_code
#         else:
#             print ">>>> Failure !"
#             print ">>>> signal is ", signal_or_code

def jpeg_to_movie(filename_pattern, path='.', fps=12, verbose=False, w=640, h=480):
    """
    Converts a series a JPEG images to a MJPEG movie.

    :param input_files: string with a wildcard such as "spam_*.jpg"
    :param output_file: string such as "output.mov"
    """
    global mencoder_executable
    global movie_name_suffix 
    if mencoder_executable is '':
        msg = "mencoder not currently installed. Cannot save to movie."
        return failure.Failure(MencoderError(msg))
    else:
        abs_filename = os.path.join(path, filename_pattern)
        jpegs = "mf://%s*.jpg" % (abs_filename)
        output_file = "%s%s.mov"  %  (abs_filename, movie_name_suffix)
        #txt_args = """%s -quiet -mf w=%d:h=%d:fps=%s:type=jpg -ovc copy -oac copy -o %s""" % (jpegs, w, h, fps, output_file) 
        txt_args = """%s -quiet -mf w=%d:h=%d:fps=%s:type=jpg -ovc lavc -lavcopts vcodec=mjpeg -oac copy -of lavf -lavfopts format=mov -o %s""" % (jpegs, w, h, fps, output_file) 
        args = txt_args.split()
        if verbose:
            print "$ mencoder %s" % (txt_args)
        try:
            deferred = utils.getProcessOutputAndValue(mencoder_executable, args, os.environ, path, reactor)
            # TODO: get rid of this and, instead, change utils.commands and add it a 
            # ProcessProtocol which supports a timeout. 
        except Exception,e:
            print "error :", sys.exc_info()
            log.critical('Cannot start the command %s. Reason: %s' % (executable, str(e.message)))
            # raise
        else:
            deferred.addErrback(cb_error, args)
            #deferred.addCallback(cb_success, args)
            return deferred

if __name__ == '__main__':
    def stop(arg):
        print "stopping reactor", arg
        reactor.stop()
    def start(filename_pattern, path, fps):
        deferred = jpeg_to_movie(filename_pattern, path, fps)
        deferred.addCallback(cb_success, args)
        #deferred.addCallback(stop)
        deferred.addErrback(stop)
    
    print "#################################################################"
    path = '.'
    #path = '/home/aalex/src/toonloop/trunk/py/toon'
    filename_pattern = "2009-03-07_05h13m49_"
    fps = 12
    reactor.callLater(0, start, filename_pattern, path, fps, True)
    print "starting reactor"
    reactor.run()

