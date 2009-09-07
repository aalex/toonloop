#!/usr/bin/env python
#
# ToonLoop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# <alexandre@quessy.net> & <le.businessman@gmail.com>
# http://www.toonloop.com
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

import os
import sys
import subprocess
from rats import purity
from twisted.internet import defer

VERBOSE = True


class ChildKilledError(Exception):
    """Raised when child is killed"""
    pass

def run_command(command_str, variables_dict={}, die_on_ctrl_c=True):
    """
    Creates and launches a process. 

    Uses subprocess to launch a process. Blocking.
    When called, might throw a OSError or ValueError.
    Throws a ChildKilledError if ctrl-C is pressed.
    """
    global VERBOSE
    retcode = None
    environment = {}
    environment.update(os.environ)
    environment.update(variables_dict)
    try:
        if VERBOSE:
            print("--------")
        print("COMMAND: %s" % (command_str))
        p = subprocess.Popen(command_str, shell=True, env=environment)
        print("PID: %s" % (p.pid))
        if VERBOSE:
            print("ENV: %s" % (str(variables_dict)))
            print("--------")
        retcode = p.wait() # blocking
        if retcode < 0:
            err = "Child was terminated by signal %d\n" % (retcode)
            sys.stderr.write(err)
        else:
            err = "Child returned %s\n" % (retcode)
            sys.stderr.write(err)
    except OSError, e:
        err = "Execution of child failed: %s\n" % (e.message)
        sys.stderr.write(err)
        retcode = 1
    except KeyboardInterrupt, e:
        if die_on_ctrl_c:
            print("Ctrl-C has been pressed in a slave terminal. Dying.")
            sys.exit(1)
        else:
            raise ChildKilledError("Ctrl-C has been pressed in the master's terminal and caught by a worker.")
    except ValueError, e:
        err = "Wrong arguments to subprocess.Popen: %s\n" % (e.message)
        sys.stderr.write(err)
        raise
    #else:
        #print("Success\n") # retrcode is p.wait() return val
    return retcode

class PureData(object):
    """
    Launches Pure Data software. 
    Blocking.
    """
    def __init__(self, rate=48000, listdev=True, inchannels=2, outchannels=2, verbose=True, driver="jack", nogui=False, blocking=True, patch=None):
        self.rate = rate
        self.listdev = listdev
        self.inchannels = inchannels
        self.outchannels = outchannels
        self.verbose = verbose
        self.driver = driver
        self.nogui = nogui
        self.blocking=blocking
        if patch is None:
            patch = os.path.join(os.path.dirname(purity.__file__), "dynamic_patch.pd")
        self.patch = patch
        # ready to go

    def start(self):
        """
        Creates args to start pd.
        Returns a deferred.
        TODO: really wait until pd is started.
        """
        command = "pd"
        if self.driver == "jack":
            command += " -jack"
        if self.verbose:
            command += " -verbose"
        command += " -r %d" % (self.rate)
        command += " -inchannels %d" % (self.inchannels)
        command += " -outchannels %d" % (self.outchannels)
        command += " %s" % (self.patch)
        run_command(command, variables_dict={}, die_on_ctrl_c=True)
        return defer.succeed(True)
        
    def stop(self):
        raise NotImplementedError("This is still to be done.")

