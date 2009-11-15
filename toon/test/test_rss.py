#!/usr/bin/env python
import unittest
from toon import rss
import datetime

class Test_Rss(unittest.TestCase):
    def test_date_format(self):
        d = datetime.datetime.now()
        f = rss._format_date(d)
        assert(f.find("GMT") != -1)
        #TODO: actually test it enough.
        
