#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Toonloop launcher
"""
import socket
import os
import sys
import gc
import warnings
# HACK to raise an exception when we get a GtkWarning
def custom_show_warning(message, category, filename, lineno, file=None, line=None):
        """ 
        Override warnings.showwarning to raise an exception if GTK cannot open the DISPLAY.
        open the display.
        """
        sys.stdout.write(warnings.formatwarning(message, category, filename, lineno))
        if "could not open display" in message:
            raise RuntimeError("Error: Could not open display. Spinic needs a $DISPLAY.")

warnings.showwarning = custom_show_warning
try:
    from twisted.internet import gtk2reactor
    gtk2reactor.install()
    import gtk
except RuntimeError, e:
    print(str(e))
    sys.exit(1)
if not os.environ.has_key("DISPLAY"):
    print("Error: Could not open display. Needs a $DISPLAY.")
    sys.exit(1)
from twisted.internet import reactor
from twisted.internet import defer
from twisted.internet import utils
from twisted.python import procutils

DEFAULT_STATESAVING_FILE = "~/.toonloop-launcher.json"

class Configuration(object):
    """
    Settings for the whole application. (except those in the config file)
    """
    def __init__(self): 
        self.data_directory = None
        self.verbose = False
        self.debug = False
        self.statesaving_file = DEFAULT_STATESAVING_FILE

class Application(object):
    """
    Application singleton which contains all the important objects.
    """
    def __init__(self, config):
        # attributes:
        self.config = config
        self.gui = None
        self.launcher = None
        self.log = None
        # action!
        self._start()

    def _start(self):
        """
        Called only once at startup.
        """
        from toonloop.launching import ProcessLauncher
        try:
            self.launcher = ProcessLauncher(app=self)
            # Logging has started in lunch after this point.
        except RuntimeError, e: # an other lunch master with the same id is running
            _exit_with_error(str(e))
        #XXX Importing those modules starts their logging.
        # it must be done once lunch master's logging has been set up
        from lunch import logger
        self.log = logger.start(name="toonloop.runner")
        from toonloop.gui import Gui
        self.gui = Gui(app=self)

def _exit_with_error(error_message):
    """
    Exits with an error dialog
    """
    from lunch import dialogs
    deferred = defer.Deferred()
    def _cb(result):
        reactor.stop()
    deferred.addCallback(_cb)
    error_dialog = dialogs.ErrorDialog(deferred, error_message)
    reactor.run()
    sys.exit(1)

def run(datadir=None, version=None):
    """
    Reads the command-line options, instanciates the application and runs the reactor.
    """
    # Instanciate the Configuration object:
    #FIXME:2010-07-28:aalex:Should not print anything before parsing command-line options
    config = Configuration()
    config.data_directory = datadir
    # parse command-line options:
    import optparse
    parser = optparse.OptionParser(usage="%prog", version=version, description=__doc__)
    parser.add_option("-v", "--verbose", action="store_true", help="Makes the logging output verbose.")
    parser.add_option("-d", "--debug", action="store_true", help="Makes the logging output even more verbose.")
    parser.add_option("-c", "--statesaving-file", type="string", help="Path to the state saving file. Defaults to %s." % (DEFAULT_STATESAVING_FILE))
    (options, args) = parser.parse_args()
    
    print("Welcome to Toonloop launcher!")
    
    # store it in the Configuration object:
    if options.config_file:
        config.statesaving_file = options.statesaving_file
    config.verbose = options.verbose
    config.debug = options.debug
    
    # instanciate the application. (might exit with error)
    app = Application(config)

    # run the reactor:
    reactor.run()
    del app
    #FIXME:2010-07-28:aalex:For some reason, the destructor is never called.
    gc.collect() # Forcing garbage collection to try to call the desctructor
    print("\nGoodbye.")
    sys.exit(0)

