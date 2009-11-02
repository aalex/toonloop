#!/usr/bin/env python
import tempfile

from rats import flatconfig 

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
        "ham"=spam
        egg="bacon and toast"
        egg="cheese"
        number =2
        number= 3
        bool_val = yes
        """)
        f.close()

    def tearDown(self):
        pass

    def test_01_parse(self):
        cp = flatconfig.ConfigParser()
        cp.read(self.file_name)
        res = cp.items()
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
        # number of elements
        expected_length = 6
        if len(res) != expected_length:
            self.fail("Expected a list of %d items, but has %s." % (expected_length, len(res)))
        dict_len = 4
        d = dict(res)
        if len(d) != dict_len:
            self.fail("Expected a dict of %d items, but has %s." % (dict_len, len(d)))

    def test_02_list(self):
        cp = flatconfig.ConfigParser()
        cp.read(self.file_name)
        # many entries with same key + int type
        numbers = cp.get_list("number", int)
        if len(numbers) != 2:
            self.fail("Expected two values with the \"number\" option name.")
        for i in [2, 3]:
            if i not in numbers:
                self.fail("Could not find value %d" % (i))

    def test_03_types(self):
        cp = flatconfig.ConfigParser()
        cp.read(self.file_name)
        # bool 
        val = cp.get("bool_val", bool)
        if val != True:
            self.fail("Expected value to be True.")


class Test_Invalid_Config_File(unittest.TestCase):
    def setUp(self):
        self.file_name = tempfile.mktemp()
        verb("File name : " + self.file_name)
        f = file(self.file_name, "w")
        f.write("""
        # this is a comment
        "ham" = 
        """)
        f.close()

    def tearDown(self):
        pass

    def test_01_no_value_for_key(self):
        cp = flatconfig.ConfigParser()
        try:
            cp.read(self.file_name)
        except flatconfig.ParsingError, e:
            pass
        else:
            res = cp.items()
            self.fail("Expected an error parsing the file but got %s" % (res))

class Test_File_Not_Found(unittest.TestCase):
    def test_not_found(self):
        cp = flatconfig.ConfigParser()
        try:
            cp.read(tempfile.mktemp())
        except flatconfig.FileNotFoundError, e:
            pass
        else:
            self.fail("Expected an error trying to read non-existant file.")
