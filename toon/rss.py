#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
RSS Generator.
"""

import datetime
import PyRSS2Gen

def test_rss():
    """
    Returns a simple RSS reponse.
    """
    items_list = []
    # ------------------------------------
    # one item
    url = "http://localhost/1"
    length = 0
    type = "txt"
    items_list.append(PyRSS2Gen.RSSItem(
             title = "Test item",
             link = "http://localhost/1",
             description = "test decription",
             guid = PyRSS2Gen.Guid("http://localhost/1"), 
             enclosure = PyRSS2Gen.Enclosure(url, length, type),
             pubDate = datetime.datetime(2003, 9, 6, 21, 31)))
    # ------------------------------------
    # the channel
    rss = PyRSS2Gen.RSS2(
        title = "ToonLoop Materials",
        link = "http://localhost/",
        description = "Stop Motion Animation Software",
        lastBuildDate = datetime.datetime.now(),
        items = items_list)
    return rss.to_xml()

if __name__ == '__main__':
    rss = test_rss()
    print rss

