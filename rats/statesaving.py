#!/usr/bin/env python
"""
State saving tools

Two json modules exist. We must use the simplejson that became 
the standard json module in Python 2.6
"""
try:
    import json # python 2.6
except ImportError:
    import simplejson as json # python 2.4 to 2.5
try:
    _tmp = json.loads
except AttributeError:
    import warnings
    import sys
    warnings.warn("Use simplejson, not the old json module.")
    sys.modules.pop('json') # get rid of the bad json module
    import simplejson as json

class StateSavingError(Exception):
    """
    Any error the Saver class can generate.
    """
    pass

def save(file_name, data):
    """
    State saving using JSON

    The data attribute is a dict of basic types.
    (``str``, ``unicode``, ``int``, ``long``, ``float``, ``bool``, ``None``)
    It can contain dicts and lists as well.
    """
    try:
        f = open(file_name, "w")
    except IOError, e:
        raise StateSavingError(e.message)
    json.dump(data, f, indent=4)
    f.close()

def load(file_name):
    try:
        f = open(file_name, "r")
    except IOError, e:
        raise StateSavingError(e.message)
    data = json.load(f)
    f.close()
    return data
        
if __name__ == "__main__":
    import tempfile
    filename = tempfile.mktemp()
    print("Loading from %s" % (filename))
    try:
        data = load(file_name)
    except StateSavingError, e:
        print(e.message)
        pass
    print("data: %s"  %(data))
    data['x'] = 2
    data['y'] = 3.14159
    data['z'] = [1,2,3,4,5,6]
    data['a'] = {'egg':'white', 'ham':'pink'}
    print("data: %s"  %(data))
    print("Saving to %s" % (file_name))
    save(file_name, data)
