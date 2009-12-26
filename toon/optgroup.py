#!/usr/bin/env python
"""
Groups of options set on the command line.
"""
VERBOSE = False

class OptionsError(Exception):
    """
    Any error related to OptionsGroup
    """
    pass

class OptionsGroup(object):
    """
    Its attributes are the its option names and values.
    """
    def set_value(self, option_name, value):
        """
        Sets a value of an option. 
        
        If value is a string, it will be casted to the type of the option
        to be set. If the option is a tuple of 4 floats, for example, it 
        will be expected to in the form "1.0,0.8,0.2,1.0" where "," is the 
        separator.

        :param option_name: Name of the attribute
        :param value: Either a string, or any Python type. 
        """
        key = option_name
        if not hasattr(self, option_name):
            raise OptionsError("No no option named %s in options group" % (key))
        else:
            attr = getattr(self, key)
            attr_type = type(attr)
            if attr_type in [int, float, str]:
                try:
                    setattr(self, key, attr_type(value))
                except ValueError, e:
                    raise OptionsError("Could not convert value %s to type %s for option %s. %s" % (value, attr_type.__name__, key, e.message))
            elif attr_type is list:
                tuple_length = len(attr)
                if type(value) is str:
                    elements = value.split(",")
                else:
                    elements = value # we assume it is a list
                if len(elements) != tuple_length:
                    raise OptionsError("Option %s should have %s elements but has %s." % (key, tuple_length, len(elements)))
                else:
                    for i in range(len(elements)):
                        element_type = type(attr[i])
                        try:
                            attr[i] = element_type(elements[i])
                        except ValueError, e:
                            raise OptionsError("Could not convert element %s (%s) to type %s for option %s. %s" % (i, elements[i], element_type.__name__, key, e.message))
            else:
                raise OptionsError("Impossible to convert option %s of group %s to type %s." % (key, group, attr_type))

if __name__ == "__main__":
    class ExampleGroup(OptionsGroup):
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
        for group, key, value in options.effect_option:
            #if VERBOSE:
            #    print("    group:%s, k:%s, v:%s" % (group, key, value))
            if group not in groups.keys():
                print("No group named %s." % (group))
            else:
                obj = groups[group]
                obj.set_value(key, value)
        #parse_options_groups(groups, options.effect_option)
        print("After:")
        for name, group in groups.items():
            print("%s: %s" % (name, group.__dict__))
