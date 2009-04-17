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
We are not yet ready to use this installation file.
But we will. 
"""
from setuptools import find_packages
from setuptools import setup

setup(
    name = "toonloop",
    version = "1.0 beta",
    author = "Alexandre Quessy",
    author_email = "alexandre@quessy.net",
    url = "http://www.toonloop.com/",
    description = "Stop Motion Software - RATS is an Anagram for Arts",
    long_description = "ToonLoop Library for OpenGL and Twisted Arts",
    install_requires = ["twisted", "OpenGL", "nevow", "pygame"],
    scripts = ["toonloop.py", "osc_send.py", "osc_receive.py"],
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    packages = [] # packages=find_packages()
    # py_modules = ["libtest"],
    # data_files=[("", ["freesansbold.ttf"])]
    )

#test_suite='nose.collector',
    
