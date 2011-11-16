#!/usr/bin/env python
"""
The Toonloop launcher GUI
"""
import os
import sys
import tempfile
if __name__ == "__main__": # just a reminder
    from twisted.internet import gtk2reactor
    gtk2reactor.install()
from twisted.internet import reactor
import gtk
from lunch import logger
from lunch.states import *

log = logger.start(name="toonloop-launcher-gui")

PACKAGE_DATA = "./data"
APP_NAME = "toonloop-launcher"
GLADE_FILE_NAME = "toonloop-launcher.glade"

def set_model_from_list(cb, items):
    """
    Set up a ComboBox or ComboBoxEntry based on a list of strings.
    
    @type cb: L{gtk.ComboBox}
    @type items: C{list}
    """
    model = gtk.ListStore(str)
    for i in items:
        model.append([i])
    cb.set_model(model)
    if type(cb) == gtk.ComboBoxEntry:
        cb.set_text_column(0)
    elif type(cb) == gtk.ComboBox:
        cell = gtk.CellRendererText()
        cb.pack_start(cell, True)
        cb.add_attribute(cell, 'text', 0)

def _get_combobox_value(widget):
    """
    Returns the current value of a GTK ComboBox widget.
    """
    index = widget.get_active()
    tree_model = widget.get_model()
    try:
        tree_model_row = tree_model[index]
    except IndexError:
        raise RuntimeError("Cannot get ComboBox's value. Its tree model %s doesn't have row number %s." % (widget, index))
    return tree_model_row[0] 

def _set_combobox_choices(widget, choices=[]):
    """
    Sets the choices in a GTK combobox.
    """
    #XXX: combo boxes in the glade file must have a space as a value to have a tree iter
    #TODO When we change a widget value, its changed callback is called...
    try:
        previous_value = _get_combobox_value(widget)
    except RuntimeError, e:
        log.error(str(e))
        previous_value = " "
    tree_model = gtk.ListStore(str)
    for choice in choices:
        tree_model.append([choice])
    widget.set_model(tree_model)
    if previous_value != " ": # we put empty spaces in glade as value, but this is not a real value, and we get rid of it.
        _set_combobox_value(widget, previous_value)

def _set_combobox_value(widget, value=None):
    """
    Sets the current value of a GTK ComboBox widget.
    """
    #XXX: combo boxes in the glade file must have a space as a value to have a tree iter
    tree_model = widget.get_model()
    index = 0
    got_it = False
    for i in iter(tree_model):
        v = i[0]
        if v == value:
            got_it = True
            break # got it
        index += 1
    if got_it:
        #widget.set_active(-1)  NONE
        widget.set_active(index)
    else:
        widget.set_active(0) # FIXME: -1)
        msg = "ComboBox widget %s doesn't have value \"%s\"." % (widget, value)
        log.debug(msg)

class Gui(object):
    """
    Main application (arguably God) class
     * Contains the main GTK window
    """
    def __init__(self, app=None):
        self.app = app
        global PACKAGE_DATA
        global GLADE_FILE_NAME
        PACKAGE_DATA = self.app.config.data_directory
        gtkbuilder_file = os.path.join(PACKAGE_DATA, GLADE_FILE_NAME)
        log.info("GtkBuilder file: %s" % (gtkbuilder_file))
        if not os.path.isfile(gtkbuilder_file):
            print("Could not find the glade file.")
            sys.exit(1)
        self.process_launcher = app.launcher
        self.builder = gtk.Builder()
        self.builder.add_from_file(gtkbuilder_file)
        self.builder.connect_signals(self)
        self.window = self.builder.get_object("main_window")
        if self.window is None:
            raise RuntimeError("Could not get the window widget.")
        from lunch import gui
        #FIXME:2010-07-28:aalex:This list of hard-coded paths is bad.
        # This first found is used as a window icon
        possible_icons = [
            "/usr/local/share/pixmaps/toonloop.png",
            "/usr/share/pixmaps/toonloop.png",
            gui.ICON_FILE,
            ]
        for icon_file in possible_icons:
            if os.path.exists(icon_file):
                large_icon = gtk.gdk.pixbuf_new_from_file(icon_file)
                self.window.set_icon_list(large_icon)
                break
        self.window.connect('delete-event', self.on_main_window_deleted)
        self.launch_toonloop_widget = self.builder.get_object("launch_toonloop")
        self.project_name_widget = self.builder.get_object("project_name")
        self.video_sources_widget = self.builder.get_object("video_sources")
        self.verbose_widget = self.builder.get_object("verbose")
        self.is_running_widget = self.builder.get_object("is_running")

        self.menu_accel_group = self.builder.get_object('accelgroup1')
        self._setup_shortcuts()
        reactor.callLater(0.01, self._start)

    def _start(self):
        """
        Initialize stuff.
        Called from the constructor at startup.
        """
        self.window.show()
        cameras_txt = "/dev/video0\n/dev/video1\n/dev/video2\ntest\nx\ndv"
        self.update_video_sources(cameras_txt)

    def update_video_sources(self, txt):
        log.debug("update_cameras_list")
        _set_combobox_choices(self.video_sources_widget, txt)
    
    def on_launch_toonloop_clicked(self, *args):
        """
        Clicked on the launch_toonloop button 
        """
        log.debug("on_launch_clicked")
        video_source = _get_combobox_value(self.video_source_widget)
        verbose = self.verbose_widget.get_active()
        log.debug("Will launch toonloop with %s" % (video_source))
        self.launch_toonloop_widget.set_sensitive(False)

    def on_about_menu_item_activated(self, *args):
        _show_about_dialog()

    def on_help_menu_item_activated(self, *args):
        pass

    def on_quit_menu_item_activated(self, *args):
        log.info("The user chose the quit menu item.")
        reactor.stop()
        # self.process_launcher.lunch_gui.confirm_and_quit()
        
    def on_main_window_deleted(self, *args):
        """
        Destroy method causes appliaction to exit
        when main window closed
        """
        # return self.process_launcher.lunch_gui.confirm_and_quit()
        reactor.stop()

    def _prepare_toonloop_command(self):
        """
        @rtype: C{str}
        """
        verbose = self.verbose_widget.get_active()
        _command = "toonloop"
        if verbose:
            _command += " --verbose"
        video_source = _get_combobox_value(self.video_source_widget)
        _command += " -d %s" % (video_source)
        project_name = "default" # TODO
        _command += " --project-home ~/Documents/toonloop/%s" % (project_name)
        log.info("$ %s" % (_command))
        return _command


