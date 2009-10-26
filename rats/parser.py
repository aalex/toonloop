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



