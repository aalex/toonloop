#!/usr/bin/env python
import tempfile

from rats import parser

from twisted.trial import unittest
from twisted.internet import defer
from twisted.internet import reactor
from twisted.python import failure

VERBOSE = False
def verb(txt):
    global VERBOSE
    if VERBOSE:
        print(txt)


class Test_Config_File(unittest.TestCase):
    def setUp(self):
        self.file_name = tempfile.mktemp()
        verb("File name : " + self.file_name)
        f = file(self.file_name, "w")
        f.write("""
        # this is a comment
        "ham" = spam
        egg = "bacon and toast"
        egg = "cheese"
        """)
        f.close()

    def tearDown(self):
        pass

    def test_01_parse(self):
        fp = parser.ConfigFileParser()
        res = fp.parse_config_file(self.file_name)
        verb("Result is %s" % (res))
        has_ham = False
        num_eggs = 0
        for k, v in res:
            if k == "egg":
                num_eggs += 1
            elif k == "ham":
                has_ham = True
        if not has_ham:
            self.fail("Expected key \"ham\" not found.")
        if num_eggs != 2:
            self.fail("Expected %s times the key %s, but found it %s times." % (2, "egg", num_eggs))
        
        if len(res) != 3:
            self.fail("Expected a list of 3 items, but has %s." % (len(d)))
        
        d = dict(res)
        if len(d) != 2:
            self.fail("Expected a dict of 2 items, but has %s." % (len(d)))

