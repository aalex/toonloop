#!/usr/bin/env python
"""
Simple configuration file/line parser.
"""
import shlex

STRIPPED = "\"' " # quotes are stripped.
ASSIGNATION_OPERATORS = "=:" # key/values are separated by those
COMMENTERS = "#"

class ParsingError(Exception):
    """
    Any error that can be raised parsing a string or file.
    """
    pass

def parse_line(txt):
    """
    Parses one line.
    Separates correctly the arguments when using quotes.
    """
    try:
        return shlex.split(txt)
    except ValueError, e:
        raise ParsingError("Error parsing text '%s': %s" %(txt, e.message))

class NoOptionError(Exception):
    """
    Exception raised when a specified option is not found.
    """
    pass

class ParsingError(Exception):
    """
    Exception raised when errors occur attempting to parse a file.
    """
    pass

class ConfigParser(object):
    """
    This prototype is not yet used.
    """
    def __init__(self):
        self.file_name = None
        self._options = []

    def read(self, file_names):
        """
        Attempt to read and parse a list of filenames, returning a list of filenames which were successfully parsed. If filenames is a string or Unicode string, it is treated as a single filename. If a file named in filenames cannot be opened, that file will be ignored. This is designed so that you can specify a list of potential configuration file locations (for example, the current directory, the user's home directory, and some system-wide directory), and all existing configuration files in the list will be read. If none of the named files exist, the ConfigParser instance will contain an empty dataset. 
        """

    def options(self):
        """
        Returns a list of options available.
        """
        return dict(self._options).keys()
    
    def get(self, option):
        """
        Get an option value.

        Might raise a KeyError if the option is not specified.
        """
        return dict(self._options)[option]

    def get_float(self, option):
        """
        A convenience method which coerces the option to a floating point number.
        """
        return float(self.get(option))
    
    def get_int(self, option):
        """
        A convenience method which coerces the option to a floating point number.
        """
        return int(self.get(option))

    def get_boolean(self, option):
        """
        A convenience method which coerces the option to a boolean.
        Note that the accepted values for the option are "1", "yes", "true", and "on", which cause this method to return True, and "0", "no", "false", and "off", which cause it to return False. These string values are checked in a case-insensitive manner. Any other value will cause it to raise ValueError.
        """
        raise NotImplementedError("TODO")
        return bool(self.get(option))

class ConfigFileParser(object):
    """
    Parses flat key-value pairs in a text file.

    Removes quotes from quoted strings.
    """
    def __init__(self):
        self.verbose = False

    def parse_config_file(self, file_name):
        """
        Parses a file for key-value pairs.
        
        Returns a list of tuples. 
        If you prefer a dict, just cast it to a dict. Like this::

          entries = dict(parse_config_file(file_name))
        
        A list of tuples makes possible the use of many times the same key.
        That is lost if you convert the result to a dict.
        """
        global STRIPPED
        global ASSIGNATION_OPERATORS
        global COMMENTERS
        #ret = {}
        ret = []
        try:
            f = open(file_name, 'r')
        except IOError, e:
            raise ParsingError("Could not open file '%s': %s" % (file_name, e.message))
        try:
            lexer = shlex.shlex(f, file_name, posix=False)
            lexer.commenters = COMMENTERS # default
            #lexer.wordchars += "|/.,$^\\():;@-+?<>!%&*`~"
            key = None
            for token in lexer:
                if token in ASSIGNATION_OPERATORS:
                    if key is None: # assignation
                        raise ParsingError("No key defined, but found '%s' in file '%s' on line %d. Try using quotes." % (token, file_name, lexer.lineno))
                else:
                    if key is None: # key
                        key = token.strip(STRIPPED)
                        #if key in ret.keys(): # TODO: allow non-uniques
                        #    raise ParsingError("Key %s already defined but found again in file '%s' on line %d." % (key, file_name, lexer.lineno))
                        if self.verbose:
                            print("Found key '%s'" % (key))
                    else: # value
                        value = token.strip(STRIPPED)
                        #ret[key] = value
                        ret.append((key, value))
                        if self.verbose:
                            print("Found value '%s' for key '%s'" % (value, key))
                        key = None
        except ValueError, e:
            raise ParsingError("Error parsing file '%s': %s" % (file_name, e.message))
        f.close()
        return ret

if __name__ == "__main__":
    # this is just for test purposes.
    # better tests are in the test/ directory
    """
    # EXAMPLE FILE:
    # comment
    hello = "ma belle"
    "you are" = "the best"
    banane = blue
    fraise = "je taime"
    """
    p = Parser()
    print(parse_line("c -l -o qwerqwe 'hello dude'"))
    print(p.parse_config_file('/var/tmp/cfg'))



