#!/usr/bin/env python
#
# ToonLoop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# http://toonloop.com
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
from twisted.web.resource import Resource
from twisted.web import server
from twisted.web import static
from twisted.internet import reactor

from docutils.core import publish_string                                                                  

class ReStructured(Resource):
    def __init__(self, filename, *a):
        self.rst = open(filename).read()

    def render(self, request):
        return publish_string(self.rst, writer_name = 'html')

if __name__ == '__main__':
    PORT = 8080
    PATH = '.'
    EXTENSION = '.rst'
    resource = static.File(PATH)
    resource.processors = {EXTENSION: ReStructured}
    resource.indexNames = ['index' + EXTENSION]
    reactor.listenTCP(PORT, server.Site(resource))
    reactor.run()
