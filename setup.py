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
Installation script for ToonLoop.

Usage : sudo python setup.py install --prefix=/usr/local
"""
from setuptools import find_packages
from setuptools import setup

setup(
    name = "toonloop",
    version = "1.0 beta",
    author = "Alexandre Quessy and Tristan Matthews",
    author_email = "alexandre@quessy.net",
    url = "http://www.toonloop.com/",
    description = "ToonLoop Realtime Stop Motion Software",
    long_description = "ToonLoop Library and application for OpenGL and Twisted Arts",
    install_requires = ["twisted", "PyOpenGL", "nevow", "numpy"], 
    # requires pygame from SVN for now.
    scripts = ["toonloop.py"], #, "osc_send.py", "osc_receive.py"],
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    packages = ['rats', 'toon'] # packages=find_packages()
    # py_modules = ["libtest"],
    # data_files=[("", ["freesansbold.ttf"])]
    )

#test_suite='nose.collector',
    
