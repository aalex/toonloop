#!/usr/bin/env python
"""
Groups of options set on the command line.
"""
VERBOSE = False

class UsageError(Exception):
    """
    Any error caused by a bad usage of the command-line.
    """
    pass

def parse_options_groups(groups, groups_options):
    """
    Changes attributes of objects passed in the groups dict according to the 
    groups_options optparse parsed option.

    Attributes that are lists are string whose elements are separated by commas. (",")

    :param groups: dict of groups objects whose keys are their names
    :param groups_options: optparse option with action="append" and nargs=3
    """
    try:
        for group, key, value in groups_options:
            if VERBOSE:
                print("    group:%s, k:%s, v:%s" % (group, key, value))
            if group in groups.keys():
                obj = groups[group]
                if hasattr(obj, key):
                    attr = getattr(obj, key)
                    attr_type = type(attr)
                    if attr_type in [int, float, str]:
                        try:
                            setattr(obj, key, attr_type(value))
                        except ValueError, e:
                            raise UsageError("Could not convert value %s to type %s for option %s of group %s. %s" % (value, attr_type.__name__, key, group, e.message))
                    elif attr_type is list:
                        tuple_length = len(attr)
                        elements = value.split(",")
                        if len(elements) != tuple_length:
                            raise UsageError("Option %s of group %s should have %s elements but has %s." % (key, group, tuple_length, len(elements)))
                        else:
                            for i in range(len(elements)):
                                element_type = type(attr[i])
                                try:
                                    attr[i] = element_type(elements[i])
                                except ValueError, e:
                                    raise UsageError("Could not convert element %s (%s) to type %s for option %s of group %s. %s" % (i, elements[i], element_type.__name__, key, group, e.message))
                    else:
                        raise Usage("Impossible to convert option %s of group %s to type %s." % (key, group, attr_type))
                else:
                    raise UsageError("No no option named %s in options group %s" % (key, group))
            else:
                raise UsageError("No options group named %s" % (group))
    except UsageError, e:
        print("ERROR: %s" % (e.message))

if __name__ == "__main__":
    class ExampleGroup(object):
        def __init__(self):
            self.i = 3
            self.f = 2.0
            self.s = "qwe"
            self.t4 = [1.0, 0.8, 0.2, 1.0]
            self.t3 = [1.0, 0.8, 0.2]
    import optparse
    VERSION = '0.1'
    parser = optparse.OptionParser(usage="%prog", version=VERSION)
    parser.add_option("-x", "--effect-option", 
        action="append", nargs=3, 
        help="Sets effect option")
    (options, args) = parser.parse_args()
    groups = {}
    groups["ex"] = ExampleGroup()
    print("Before:")
    for name, group in groups.items():
        print("    %s: %s" % (name, group.__dict__))
    print("effect_option: %s" % (options.effect_option))
    if options.effect_option is not None:
        parse_options_groups(groups, options.effect_option)
        print("After:")
        for name, group in groups.items():
            print("%s: %s" % (name, group.__dict__))

