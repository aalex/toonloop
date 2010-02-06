#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import gtk.glade
from twisted.internet import gtk2reactor
gtk2reactor.install()
from twisted.internet import reactor

__version__ = "0.1.0"
PACKAGE_DATA = "."
APP_NAME = "toonloop"

class Gui(object):
    """
    Main application (arguably God) class
     * Contains the main GTK window
    """
    def __init__(self):
        glade_file = os.path.join(PACKAGE_DATA, 'toonloop.glade')
        if os.path.isfile(glade_file):
            glade_path = glade_file
        else:
            print "Could not find the glade file."
            sys.exit(1)
        self.widgets = gtk.glade.XML(glade_path, domain=APP_NAME)
        self.window = self.widgets.get_widget("main_window")
        self.window.connect('delete-event', self.on_main_window_deleted)
        self.window.show()
   
    def on_main_window_deleted(self, *args):
        """
        Destroy method causes appliaction to exit
        when main window closed
        """
        reactor.stop()

if __name__ == "__main__":
    gui = Gui()
    reactor.run()
