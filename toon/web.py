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
Toonloop Web server for files and RSS feeds.
"""
import os
import sys

from twisted.internet import reactor
from nevow import static
from nevow import appserver
from nevow import rend

from toon import rss
from toon import rst

class Index(rst.ReStructured, rend.Page):
    """
    Class representing the root (/) of the web server. 
    """
    addSlash = True

    def __init__(self, toonloop, **kwargs):
        self.static_files_path = os.curdir
        self.index_file_path = os.path.join(os.path.dirname(__file__), 'data', 'index.rst')
        self.port = 8000
        self.__dict__.update(**kwargs)
        
        print "Static files root:", self.static_files_path
        try:
            self.toonloop = toonloop
        except AttributeError, e:
            print 'web_server: Index', e.message
            self.toonloop = None
        try:
            rst.ReStructured.__init__(self, self.index_file_path)
        except IOError, e:
            print "Error reading rst file.", e.message, self.index_file_path
        # child_* attributes serve some static files
        # TODO: RSS feed and web form
        self.child_files = static.File(self.static_files_path) # os.path.join
        self.child_data = static.File(os.path.join(os.path.dirname(__file__), 'data'))
        self.child_rss = rss.RSSPage(port=self.port, root=self.static_files_path)
    
    def renderHTTP(self, request):
        """
        Renders the / page
        """
        return rst.ReStructured.render(self, request)

def start(toonloop, port=8000, **kwargs): 
    """
    Called from the main application to start the web UI.
    Config argmuments ARE overriden from the application.

    :param toonloop: The application
    :param port: web server port
    """
    # These are default values and are overriden in the main toonloop script.
    web_config = {
        'static_files_path':os.path.expanduser("~/Documents/toonloop"),
         'index_file_path':os.path.join(os.path.dirname(__file__), 'data', 'index.rst'), 
        'port':port
    }
    web_config.update(kwargs)
    #Index.static_files_path = web_config['static_files_path']
    #Index.index_file_path = web_config['index_file_path']
    site = appserver.NevowSite(Index(toonloop, **web_config))
    if toonloop.config.verbose:
        print 'Starting web server on port', port
        print 'Static Documentation Files Path : ', web_config['index_file_path']
    reactor.listenTCP(port, site)

if __name__ == '__main__':
    # print sys.path
    start(None)
    try:
        reactor.run()
    except:
        raise

