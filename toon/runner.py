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
"""
Main runner of the Toonloop application.
"""
__version__ = "1.2.0" # MUST ALSO CHANGE IT IN setup.py

import sys
import os
import pygame 
import optparse
try:
    from twisted.internet import gtk2reactor
    gtk2reactor.install() # has to be done before importing reactor
    GTK_LOADED = True
except Exception, e:
    GTK_LOADED = False
    print("Could not load GTK: %s" % (e))
from twisted.internet import reactor
from twisted.internet import error

# the core: 
from rats import render
from toon import core # Configuration, Toonloop, ToonloopError
from toon import optgroup

def exit_with_error(message):
    """
    Exits with error.
    If possible, shows a GTK error dialog.
    """
    print(message)
    if GTK_LOADED:
        from toon import dialogs
        d = dialogs.ErrorDialog.create(message)
        def _cb(result):
            if reactor.running:
                reactor.stop()
            sys.exit(1)
        d.addCallback(_cb)
        if not reactor.running:
            reactor.run()
    else:
        sys.exit(1)

def run():
    """
    Starts the application, reading the command-line arguments.
    """
    parser = optparse.OptionParser(usage="%prog", version='Toonloop ' + str(__version__)) 
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
    parser.add_option("-F", "--fullscreen", action="store_true", \
        help="Displays in fullscreen mode at startup.") # default=False
    parser.add_option("-t", "--intervalometer-rate-seconds", type="float",  \
        help="Sets intervalometer interval in seconds.", default=30.0)
    parser.add_option("-c", "--config-file", type="string",  \
        help="Path to config file.", default=os.path.expanduser("~/.toonloop.json"))
    parser.add_option("-H", "--toonloop-home", type="string",  \
        help="Path to saved files.")
    parser.add_option("-i", "--intervalometer-on", \
        dest="intervalometer_on", action="store_true", \
        help="Starts the intervalometer at startup.")
    parser.add_option("-n", "--enable-effects", \
        action="store_true", \
        help="Enables GLSL shader effects.")
    parser.add_option("-x", "--options-group", 
        action="append", nargs=3, 
        help="Sets option from an option group.")
    parser.add_option("-e", "--intervalometer-enabled", \
        dest="intervalometer_enabled", action="store_true", \
        help="Enables/disables the use of the intervalometer.", default=True)
    parser.add_option("-w", "--image-width", type="int", \
        help="Width of the images grabbed from the camera. Using this flag also sets the height, with a 4:3 ratio.")
    (options, args) = parser.parse_args()
    pygame.init()
    config = core.Configuration() # all the config is in this object.
    config_dict = config.__dict__
    if options.config_file:
        config_dict["config_file"] = options.config_file
    # XXX: Loading from the json state saving file might cause bugs that 
    # are hard to find.
    config.load() # updates the config dict with values serialized before.
    if options.list_options:
        print("""Use Toonloop options with -o flag :
        toonloop -o [name] [value]""")
        print("Toonloop options and their current values :")
        config.print_values()
        sys.exit(0)
    if options.toonloop_home:
        config_dict['toonloop_home'] = options.toonloop_home
    if options.fullscreen:
        config_dict["display_fullscreen"] = options.fullscreen
    if options.enable_effects:
        config_dict["effects_enabled"] = True
    if options.image_width:
        config_dict['image_width'] = options.image_width
        config_dict['image_height'] = options.image_width * 3 / 4 # fixed aspect ratio
    # options that have default values:
    config_dict['video_device'] = options.device #FIXME
    config_dict['intervalometer_rate_seconds'] = options.intervalometer_rate_seconds
    config_dict['intervalometer_on'] = options.intervalometer_on == True
    config_dict['intervalometer_enabled'] = options.intervalometer_enabled
    config_dict['verbose'] = options.verbose == True

    print("Toonloop - Version " + str(__version__))
    print("Copyright 2008 Alexandre Quessy & Tristan Matthews")
    print("Released under the GNU General Public License")
    print("Using video device %d" % options.device)
    print("Press h for usage and instructions.")
    print("Press i for informations and statistics.")
    if config_dict['verbose']:
        print("Started in verbose mode.")
    if options.option:
        if config.verbose:
            print('options: %s' % (options.option))
        for k, v in options.option:
            try:
                kind = config.set(k, v)
                if config.verbose:
                    print("OPTION \"%s\" : %s       %s" % (k, v, kind))
            except KeyError, e:
                exit_with_error("Error. No such Toonloop option : %s" % (e.message))
            except Exception, e:
                print(sys.exc_info())
                exit_with_error('Error with Toonloop option :', e.message)
    try:
        toonloop = core.Toonloop(config)
        if options.verbose:
            toonloop.print_help()
    except core.ToonloopError, e:
        exit_with_error("Exiting toonloop with error: %s" % (e))
        return
    else:
        print("Congratulations ! Toonloop started gracefully.")
    pygame_timer = render.Renderer(toonloop, False) # not verbose !  options.verbose
    pygame_timer.desired_fps = options.fps
    # optgroups must be set once toonloop has been initialized.
    if options.options_group is not None:
        for group, key, value in options.options_group:
            try:
                toonloop.set_option_in_group(group, key, value)
            except ToonloopError, e:
                print(e.message)
            except optgroup.OptionsError, e:
                print(e.message)
    try:
        reactor.run()
    except KeyboardInterrupt:
        pass # will exit on ctrl-c
    print("Exiting toonloop")
    sys.exit(0)
