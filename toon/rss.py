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
"""
RSS Feed Generation

"""
import os
import glob
import datetime
import pprint

from nevow import rend
from nevow.inevow import IRequest

def _format_date(dt):
    """
    Converts a datetime into an RFC 822 formatted date
    
    Input date must be in GMT.
    
    Source : PyRSS2Gen - A Python library for generating RSS 2.0 feeds.
    :author: Andrew Dalke <dalke@dalkescientific.com>
   
    Looks like:
      Sat, 07 Sep 2002 00:00:01 GMT
    Can't use strftime because that's locale dependent

    Isn't there a standard way to do this for Python?  The
    rfc822 and email.Utils modules assume a timestamp.  The
    following is based on the rfc822 module.
    """
    return "%s, %02d %s %04d %02d:%02d:%02d GMT" % (
        ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"][dt.weekday()],
        dt.day,
        ["Jan", "Feb", "Mar", "Apr", "May", "Jun",
         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"][dt.month - 1],
        dt.year, dt.hour, dt.minute, dt.second)

class Item(object):
    """
    Generates a RSS item
    """
    def __init__(self, **kwargs):
        self.title = ''
        self.link = '' 
        self.description = ''
        self.enclosure_url = '' 
        self.enclosure_length = 0 
        self.enclosure_type = 'text/txt'
        self.guid = '' 
        self.pub_date = datetime.datetime.now()
        #print "pub_date", self.pub_date
        self.__dict__.update(kwargs)

    def __str__(self):
        return """
            <item>
              <title>%s</title>
              <link>%s</link>
              <description>%s</description>
              <enclosure url="%s" length="%s" type="%s">
              </enclosure>
              <guid isPermaLink="true">%s</guid>
              <pubDate>%s</pubDate>
            </item>
        """ % (
        self.title,
        self.link, 
        self.description, 
        self.enclosure_url, 
        self.enclosure_length, 
        self.enclosure_type, 
        self.guid, 
        _format_date(self.pub_date))

class Channel(object):
    def __init__(self, **kwargs):
        self.title = 'ToonLoop'
        self.link = 'http://localhost/'
        self.description = 'Stop Motion Animation Software'
        self.last_build_date = datetime.datetime.now()
        # print "pub_date", self.last_build_date
        self.generator = 'ToonLoop'
        self.docs = 'http://toonloop.com'
        self.items = []
        self.__dict__.update(kwargs)

    def __str__(self):
        items = ""
        for item in self.items:
            items += str(item)
        return """<?xml version="1.0" encoding="us-ascii"?>
        <rss version="2.0">
        <channel>
          <title>%s</title>
          <link>%s</link>
          <description>%s</description>
          <lastBuildDate>%s</lastBuildDate>
          <generator>%s</generator>
          <docs>%s</docs>
          %s
        </channel>
        </rss>  
        """ % (
        self.title,
        self.link,
        self.description,
        _format_date(self.last_build_date),
        self.generator,
        self.docs,
        items)

class RSSPage(rend.Page): # page.Element):
    def __init__(self, **kwargs):
        self.root = os.path.expanduser("~/Documents/toonloop")
        self.port = 8000
        self.__dict__.update(kwargs)

    def renderHTTP(self, context):
        #TODO: make path aware of the host name it should be
        # print "HTTP Request:", request
        # print request
        # pprint.pprint(request.__dict__)
        # print 'tag- rss page'
        # pprint.pprint(request.tag.__dict__)
        # print 'parent'
        # pprint.pprint(request.parent.__dict__)
        request = IRequest(context)
        server_address = "http://%s"  % (request.getHeader('host'))

        channel = Channel()
        movie_files = glob.glob("%s/*/movie_*.mov" % (self.root))
        for f in movie_files:
            file_name = os.path.split(f)[1]
            project_name = os.path.dirname(f).split("/")[-1]
            #print "project:", project_name
            link = "%s/files/%s/%s" % (server_address, project_name, file_name)
            channel.items.append(Item(title=file_name, enclosure_url=link, link=link, guid=file_name, enclosure_type="video/x-msvideo"))
        return str(channel)

if __name__ == '__main__':
    channel = Channel()
    channel.items.append(Item(title='Hello', link='http://localhost/1', guid='1', enclosure_url='http://localhost/download/1'))
    print str(channel)

