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
Installation script for Toonloop.
---------------------------------

Usage::
  python setup.py build
  sudo python setup.py install --prefix=/usr/local

For developpers::
  sudo python setup.py develop --prefix=/usr/local
  sudo python setup.py develop --prefix=/usr/local --uninstall

To make package::
  python setup.py sdist
"""
from setuptools import find_packages
from setuptools import setup
import sys
import subprocess

__version__ = "1.2.0"

setup(
    name = "toonloop",
    version = __version__, # MUST ALSO BE CHANGED IN toon/runner.py 
    author = "Alexandre Quessy and Tristan Matthews",
    author_email = "alexandre@quessy.net",
    url = "http://www.toonloop.com/",
    description = "Toonloop Live Stop Motion Software",
    long_description = """Toonloop stop motion tool and the rats package for OpenGL and Twisted Arts. 
    Requires: twisted, pygame 1.9, PyOpenGL, nevow and numpy.""",
    install_requires = [], # "twisted", "PyOpenGL", 
    # requires pygame 1.9,1 or later
    scripts = ["toonloop"], #, "scripts/glslang-run"], 
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    packages = ["rats", "toon", "toon/data", "toon/effects"],
    package_data = {
        "":["*.cfg", "*.png", "*.jpg", "*.pd"]
    }
    )

if sys.argv[1] == "build":
    command_icon = "convert -geometry 48x48 -background none toonloop.svg toonloop.png"
    print(command_icon)
    retcode = subprocess.call(command_icon, shell=True)
    print("Command returned %s" % (retcode))
    command_man = """help2man --no-info --include=man_toonloop.txt --name="The Toonloop Live Stop Motion Tool" ./toonloop > toonloop.1"""
    print(command_man)
    retcode = subprocess.call(command_man, shell=True)
    print("Command returned %s" % (retcode))
