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

setup(
    name = "toonloop",
    version = "1.0.1",
    author = "Alexandre Quessy and Tristan Matthews",
    author_email = "alexandre@quessy.net",
    url = "http://www.toonloop.com/",
    description = "ToonLoop Live Stop Motion Software",
    long_description = """ToonLoop stop motion tool and the rats package for OpenGL and Twisted Arts. 
    Requires: twisted, pygame 1.9, PyOpenGL, nevow and numpy.""",
    install_requires = [], # "twisted", "PyOpenGL", "nevow"], # , "numpy"], 
    # requires pygame from SVN for now.
    scripts = ["toonloop"], #, "scripts/theitem", "scripts/purity-example.py"], # pd-purepy, purepy.py #, "osc_send.py", "osc_receive.py"],
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    packages = ['rats', 'toon', 'toon/data'],
    package_data = {
        "":["*.ttf", "*.rst", "*.png", "*.jpg", "*.pd"]
    }
    )

#test_suite='nose.collector',
#      data_files = [
#         ('share/man/man1', [
#             'scripts/send_osc.1',
#             'scripts/dump_osc.1',
#         ]),
#     ],
 
