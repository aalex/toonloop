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
Main runner of the ToonLoop application.
"""
 
__version__ = "1.0.3" # MUST ALSO CHANGE IT IN setup.py

import sys
import os
import pygame 
import optparse

from twisted.internet import reactor
from twisted.internet import error

# the core: 
from rats import render
from toon import core # Configuration, ToonLoop, ToonLoopError

def run():
    """
    Starts the application, reading the command-line arguments.
    """
        # self.intervalometer_on = False
        # self.intervalometer_enabled = False
        # self.intervalometer_rate_seconds = 30.0 # in seconds
    EPILOG="ToonLoop is a live stop motion performance tool. The objective is to spread its use for teaching new medias to children and to give a professional tool for movie creators. In the left window, you can see what is seen by the live camera. In the right window, it is the result of the stop motion loop."
    parser = optparse.OptionParser(usage="%prog", version='ToonLoop ' + str(__version__), \
        epilog=EPILOG)
    # + " \n\n"  __doc__
    parser.add_option("-d", "--device", dest="device", type="int", \
        help="Specifies V4L2 device to grab image from. Expects an integer such as 0, 1 or 2.", default=0)
    parser.add_option("-v", "--verbose", dest="verbose", action="store_true", \
        help="Sets the output to verbose.")
    # most important, the configuration options:
    parser.add_option("-o", "--option", action="append", nargs=2, \
        help="Sets any toonloop option by name. See list-options for a list of possible options. Example : -o project_name example")
    parser.add_option("-l", "--list-options", \
        dest="list_options", action="store_true", \
        help="Prints the list of options and exits.") 
    # other args:
    parser.add_option("-f", "--fps", type="int", \
        help="Sets the rendering frame rate. Default is 30 FPS.", default=30)
    parser.add_option("-t", "--intervalometer-rate-seconds", type="float",  \
        help="Sets intervalometer interval in seconds.", default=30.0)
    parser.add_option("-c", "--config-file", type="string",  \
        help="Path to config file.", default=os.path.expanduser("~/.toonloop.json"))
    parser.add_option("-H", "--toonloop-home", type="string",  \
        help="Path to saved files.")
    parser.add_option("-i", "--intervalometer-on", \
        dest="intervalometer_on", action="store_true", \
        help="Starts the intervalometer at startup.") # default=False
    parser.add_option("-e", "--intervalometer-enabled", \
        dest="intervalometer_enabled", action="store_true", \
        help="Enables/disables the use of the intervalometer.", default=True)
    parser.add_option("-w", "--image-width", type="int", \
        help="Width of the images grabbed from the camera. Using this flag also sets the height, with a 4:3 ratio.")
    (options, args) = parser.parse_args()
    print("ToonLoop - Version " + str(__version__))
    print("Copyright 2008 Alexandre Quessy & Tristan Matthews")
    print("Released under the GNU General Public License")
    print("Using video device %d" % options.device)
    print("Press h for usage and instructions.")
    print("Press i for informations and statistics.")
    pygame.init()
    config = core.Configuration() # all the config is in this object.
    config_dict = config.__dict__
    if options.config_file:
        config_dict["config_file"] = options.config_file
    config.load() # updates the config dict with values serialized before.
    if options.toonloop_home:
        config_dict['toonloop_home'] = options.toonloop_home
    if options.image_width:
        config_dict['image_width'] = options.image_width
        config_dict['image_height'] = options.image_width * 3 / 4 # fixed aspect ratio
    # options that have default values:
    config_dict['video_device'] = options.device
    config_dict['intervalometer_rate_seconds'] = options.intervalometer_rate_seconds
    config_dict['intervalometer_on'] = options.intervalometer_on == True
    config_dict['intervalometer_enabled'] = options.intervalometer_enabled
    config_dict['verbose'] = options.verbose == True

    if config_dict['verbose']:
        print("Started in verbose mode.")
    if options.option:
        print('options: %s' % (options.option))
        for k, v in options.option:
            try:
                kind = config.set(k, v)
                print("OPTION \"%s\" : %s       %s" % (k, v, kind))
            except KeyError, e:
                print("Error. No such ToonLoop option : %s" % (e.message))
                sys.exit(1)
                #raise core.ToonLoopError('No such ToonLoop option :', e.message)
            except Exception, e:
                print(sys.exc_info())
                raise core.ToonLoopError('Error with ToonLoop option :', e.message)
    if options.list_options:
        print("""Use ToonLoop options with -o flag :
        toonloop -o [name] [value]""")
        print("ToonLoop options and their current values :")
        config.print_values()
        print("\nExiting.")
        sys.exit(0)
    try:
        toonloop = core.ToonLoop(config)
        if options.verbose:
            toonloop.print_help()
    except core.ToonLoopError, e:
        print(str(e.message))
        print("Exiting toonloop with error")
        sys.exit(1)
    else:
        print("Congratulations ! ToonLoop started gracefully.")
    pygame_timer = render.Renderer(toonloop, False) # not verbose !  options.verbose
    pygame_timer.desired_fps = options.fps
    try:
        reactor.run()
    except KeyboardInterrupt:
        pass # will exit on ctrl-c
    print("Exiting toonloop")
    try:
        reactor.stop() # just in case.
    except error.ReactorNotRunning, e:
        pass
    sys.exit(0)

