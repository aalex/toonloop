#!/usr/bin/env python
"""
Simple configuration file/line parser.
"""
import shlex

class FileNotFoundError(Exception):
    """
    Exception raised when a config file is not found.
    """
    pass

class NoSuchOptionError(Exception):
    """
    Exception raised when a specified option is not found.
    """
    pass

class ParsingError(Exception):
    """
    Exception raised when errors occur attempting to parse a file.
    """
    pass

# def parse_line(txt):
#     """
#     Parses one line.
#     Separates correctly the arguments when using quotes.
#     """
#     try:
#         return shlex.split(txt)
#     except ValueError, e:
#         raise ParsingError("Error parsing text '%s': %s" %(txt, e.message))

class ConfigParser(object):
    """
    Parses flat key-value pairs in a text file.
    
    Removes quotes from quoted strings.
    
    A list of tuples makes possible the use of many times the same key.
    That is lost if you convert the result to a dict.

    This ConfigParser is different from the one from the ConfigParser module, 
    since it does not use the concept of sections. Also, there can be many options
    with the same key name. This class is partly insprired from ConfigParser. 
    """
    STRIPPED = "\"' " # quotes are stripped.
    ASSIGNATION_OPERATORS = "=:" # key/values are separated by those
    COMMENTERS = "#"

    def __init__(self):
        self.file_name = None
        self._options = [] # list of (key, value) tuples
        self.verbose = False

    def read(self, file_name):
        """
        Attempts to read a file and parse its configuration entries.
        It might throw a ParsingError.
        """
        self._read(file_name)

    def options(self):
        """
        Returns a list of options available.
        """
        return dict(self._options).keys()

    def has_option(self, option):
        """
        Checks for the existence of a given option.
        """
        return option in self.options()
    
    def get(self, option, cast=str):
        """
        Get an option value as a string.

        The cast argument allows you to cast the type of the value to int, float or bool.
        Just pass it a type. 

        Might raise a NoSuchOptionError if the option is not found.
        Might raise a ValueError if the value is not of the given type.

        If a key is defined many times, behavious is undefined. It it likely to return 
        the last item with that key.

        Note that the accepted values for the option are "1", "yes", "true", and "on", which cause this method to return True, and "0", "no", "false", and "off", which cause it to return False. These string values are checked in a case-insensitive manner. Any other value will cause it to raise ValueError.
        """
        try:
            value = dict(self._options)[option]
        except KeyError, e:
            raise NoSuchOptionError("Could not find option named %s." % (e.message))
        return self.cast(value, cast)

    _boolean_states = {'1': True, 'yes': True, 'true': True, 'on': True,
                       '0': False, 'no': False, 'false': False, 'off': False}

    def get_list(self, option, cast=str):
        """
        Returns a list of values for a given key name.
        """
        ret = []
        for k, v in self._options:
            if k == option:
                ret.append(self.cast(v, cast))
        if len(ret) == 0:
            raise NoSuchOptionError("Could not find option named %s." % (option))
        return ret
    
    def cast(self, value, cast):
        """
        Casts a value to a given type. 
        Type can be int, float, str or bool.
        """
        if cast in [float, int]:
            return cast(value)
        elif cast is bool:
            if value.lower() not in self._boolean_states:
                raise ValueError("Value %s is not a valid boolean." % (value))
            return self._boolean_states[value.lower()]
        else: # str
            return value

    def items(self):
        """
        Return a list of tuples with (name, value) for each option.
        """
        return self._options

    def _read(self, file_name):
        self.file_name = file_name
        self._options = []
        try:
            f = open(file_name, 'r')
        except IOError, e:
            raise FileNotFoundError("Could not open file '%s': %s" % (file_name, e.message))
        try:
            lexer = shlex.shlex(f, file_name, posix=False)
            lexer.commenters = self.COMMENTERS # default
            #lexer.wordchars += "|/.,$^\\():;@-+?<>!%&*`~"
            key = None
            for token in lexer:
                if token in self.ASSIGNATION_OPERATORS:
                    if key is None: # assignation
                        raise ParsingError("No key defined, but found '%s' in file '%s' on line %d. Try using quotes." % (token, file_name, lexer.lineno))
                else:
                    if key is None: # key
                        key = token.strip(self.STRIPPED)
                        #if key in ret.keys(): # TODO: allow non-uniques
                        #    raise ParsingError("Key %s already defined but found again in file '%s' on line %d." % (key, file_name, lexer.lineno))
                        if self.verbose:
                            print("Found key '%s'" % (key))
                    else: # value
                        value = token.strip(self.STRIPPED)
                        #ret[key] = value
                        self._options.append((key, value))
                        if self.verbose:
                            print("Found value '%s' for key '%s'" % (value, key))
                        key = None
            if key is not None:
                raise ParsingError("No value for key %s" % (key))
        except ValueError, e:
            raise ParsingError("Error parsing file '%s': %s" % (file_name, e.message))
        f.close()
        return self._options

# if __name__ == "__main__":
#     # this is just for test purposes.
#     # better tests are in the test/ directory
#     """
#     # EXAMPLE FILE:
#     # comment
#     hello = "ma belle"
#     "you are" = "the best"
#     banane = blue
#     fraise = "je taime"
#     """
#     print(parse_line("c -l -o qwerqwe 'hello dude'"))
# 
#     p = ConfigParser()
#     print(p.read('/var/tmp/cfg'))


