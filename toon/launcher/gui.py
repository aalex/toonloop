#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.

"""
Scenic GTK GUI.
"""

import os
import smtplib
import gtk.gdk
import webbrowser
from twisted.internet import reactor
from twisted.internet import defer
from twisted.internet import task
from twisted.python.reflect import prefixedMethods
from scenic import configure
from scenic import process # just for constants
from scenic import dialogs
from scenic import glade
from scenic import preview
from scenic import network
from scenic import communication
from scenic.devices import cameras
from scenic.devices import networkinterfaces
from scenic.internationalization import _

INVITE_TIMEOUT = 10
ONLINE_HELP_URL = "http://svn.sat.qc.ca/trac/scenic/wiki/Documentation"
ONE_LINE_DESCRIPTION = """Scenic is a telepresence software oriented for live performances."""
ALL_SUPPORTED_SIZE = [ # by milhouse video
    "924x576",
    "768x480",
    "720x480",
    "704x480",
    "704x240",
    "640x480",
    "352x240",
    "320x240",
    "176x120"
    ]

LICENSE_TEXT = _("""Scenic
Copyright (C) 2009 Society for Arts and Technology (SAT)
http://www.sat.qc.ca
All rights reserved.

This file is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Scenic is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Scenic.  If not, see <http://www.gnu.org/licenses/>.""")

PROJECT_WEBSITE = "http://svn.sat.qc.ca/trac/scenic"

AUTHORS_LIST = [
    'Alexandre Quessy <alexandre@quessy.net>',
    'Tristan Matthews <tristan@sat.qc.ca>',
    'Simon Piette <simonp@sat.qc.ca>',
    u'Étienne Désautels <etienne@teknozen.net>',
    ]

COPYRIGHT_SHORT = _("Copyright 2009-2010 Society for Arts and Technology")

def _get_key_for_value(dictionnary, value):
    """
    Returns the key for a value in a dict.
    @param dictionnary: dict
    @param value: The value.
    """
    return dictionnary.keys()[dictionnary.values().index(value)]

def _get_combobox_value(widget):
    """
    Returns the current value of a GTK ComboBox widget.
    """
    index = widget.get_active()
    tree_model = widget.get_model()
    try:
        tree_model_row = tree_model[index]
    except IndexError, e:
        raise RuntimeError("ComboBox widget %s doesn't have value with index %s." % (widget, index))
    #except TypeError, e:
    #    raise RuntimeError("%s is not a ComboBox widget" % (widget))
    return tree_model_row[0] 

def _set_combobox_choices(widget, choices=[]):
    """
    Sets the choices in a GTK combobox.
    """
    #XXX: combo boxes in the glade file must have a space as a value to have a tree iter
    #TODO When we change a widget value, its changed callback is called...
    previous_value = _get_combobox_value(widget)
    tree_model = gtk.ListStore(str)
    for choice in choices:
        tree_model.append([choice])
    widget.set_model(tree_model)
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
        msg = "ComboBox widget %s doesn't have value %s." % (widget, value)
        print msg

#videotestsrc legible name:
VIDEO_TEST_INPUT = "Color bars"

# GUI legible value to milhouse value mapping:
VIDEO_CODECS = {
    "h.264": "h264",
    "h.263": "h263",
    "Theora": "theora",
    "MPEG4": "mpeg4"
    }
AUDIO_CODECS = {
    "Raw": "raw",
    "MP3": "mp3",
    "Vorbis": "vorbis",
    }
AUDIO_SOURCES = {
    "JACK": "jackaudiosrc",
    "Test sound": "audiotestsrc"
    }
# min/max:
VIDEO_BITRATE_MIN_MAX = {
    "h.264": [2.0, 16.0],
    "MPEG4": [0.5, 4.0],
    "h.263": [0.5, 4.0],
    }
# standards:
VIDEO_STANDARDS = ["NTSC", "PAL"]

def format_contact_markup(contact):
    """
    Formats a contact for the Adressbook GTK widget.
    @param contact: A dict with keys "name" and "address"
    @rtype: str
    @return: Pango markup for the TreeView widget.
    """
    auto_accept = ""
    if contact["auto_accept"]:
        auto_accept = "\n  " + _("Automatically accept invitations")
    return "<b>%s</b>\n  IP: %s%s" % (contact["name"], contact["address"], auto_accept) 


class Gui(object):
    """
    Graphical User Interface
     * Contains the main GTK window.
     * And some dialogs.
    """
    def __init__(self, app, kiosk_mode=False, fullscreen=False):
        self.app = app
        self.kiosk_mode_on = kiosk_mode
        self._inviting_timeout_delayed = None
        widgets_tree = glade.get_widgets_tree()
        
        self._widgets_changed_by_user = True # if False, we are changing some widget's value programmatically.
        # connects callbacks to widgets automatically
        glade_signal_slots = {}
        for method in prefixedMethods(self, "on_"):
            glade_signal_slots[method.__name__] = method
        widgets_tree.signal_autoconnect(glade_signal_slots)
        
        # Get all the widgets that we use
        self.main_window = widgets_tree.get_widget("main_window")
        self.main_window.connect('delete-event', self.on_main_window_deleted)
        self.main_window.set_icon_from_file(os.path.join(configure.PIXMAPS_DIR, 'scenic.png'))
        self.main_tabs_widget = widgets_tree.get_widget("mainTabs")
        self.system_tab_contents_widget = widgets_tree.get_widget("system_tab_contents")
        self.main_window.connect("window-state-event", self.on_window_state_event)
        
        # ------------------------------ dialogs:
        # confirm_dialog: (a simple yes/no)
        self.confirm_dialog = dialogs.ConfirmDialog(parent=self.main_window)

        # calling_dialog: (this widget is created and destroyed really often !!
        self.calling_dialog = None
        
        # invited_dialog:
        self.invited_dialog = dialogs.InvitedDialog(parent=self.main_window)
        
        # edit_contact_window:
        self.edit_contact_window = widgets_tree.get_widget("edit_contact_window")
        self.edit_contact_window.set_transient_for(self.main_window) # child of main window
        self.edit_contact_window.connect('delete-event', self.edit_contact_window.hide_on_delete)
        
        # fields in the edit contact window:
        self.contact_name_widget = widgets_tree.get_widget("contact_name")
        self.contact_addr_widget = widgets_tree.get_widget("contact_addr")
        self.contact_auto_accept_widget = widgets_tree.get_widget("contact_auto_accept")
        
        # -------------------- main window widgets:
        # invite button:
        self.invite_label_widget = widgets_tree.get_widget("invite_label")
        self.invite_icon_widget = widgets_tree.get_widget("invite_icon")
        
        # addressbook buttons:
        self.edit_contact_widget = widgets_tree.get_widget("edit_contact")
        self.add_contact_widget = widgets_tree.get_widget("add_contact")
        self.remove_contact_widget = widgets_tree.get_widget("remove_contact")
        self.invite_contact_widget = widgets_tree.get_widget("invite_contact")
        # treeview:
        self.contact_list_widget = widgets_tree.get_widget("contact_list")
        # position of currently selected contact in list of contact:
        self.selected_contact_row = None
        self.select_contact_index = None

        # Summary text view:
        self.info_peer_widget = widgets_tree.get_widget("info_peer")
        self.info_send_video_widget = widgets_tree.get_widget("info_send_video")
        self.info_send_audio_widget = widgets_tree.get_widget("info_send_audio")
        self.info_receive_video_widget = widgets_tree.get_widget("info_receive_video")
        self.info_receive_audio_widget = widgets_tree.get_widget("info_receive_audio")
        self.info_ip_widget = widgets_tree.get_widget("info_ip")
        self.info_receive_midi_widget = widgets_tree.get_widget("info_receive_midi")
        self.info_send_midi_widget = widgets_tree.get_widget("info_send_midi")

        # video
        self.video_capture_size_widget = widgets_tree.get_widget("video_capture_size")
        self.video_display_widget = widgets_tree.get_widget("video_display")
        self.video_bitrate_widget = widgets_tree.get_widget("video_bitrate")
        self.video_source_widget = widgets_tree.get_widget("video_source")
        self.video_codec_widget = widgets_tree.get_widget("video_codec")
        self.video_fullscreen_widget = widgets_tree.get_widget("video_fullscreen")
        self.video_view_preview_widget = widgets_tree.get_widget("video_view_preview")
        self.video_deinterlace_widget = widgets_tree.get_widget("video_deinterlace")
        self.aspect_ratio_widget = widgets_tree.get_widget("aspect_ratio")
        self.v4l2_input_widget = widgets_tree.get_widget("v4l2_input")
        self.v4l2_standard_widget = widgets_tree.get_widget("v4l2_standard")
        self.video_jitterbuffer_widget = widgets_tree.get_widget("video_jitterbuffer")
        # video preview:
        self.preview_area_widget = widgets_tree.get_widget("preview_area")
        self.preview_area_x_window_id = None
        self.preview_in_window_widget = widgets_tree.get_widget("preview_in_window")
        
        # audio
        self.audio_source_widget = widgets_tree.get_widget("audio_source")
        self.audio_codec_widget = widgets_tree.get_widget("audio_codec")
        self.audio_jack_icon_widget = widgets_tree.get_widget("audio_jack_icon")
        self.audio_jack_state_widget = widgets_tree.get_widget("audio_jack_state")
        self.audio_numchannels_widget = widgets_tree.get_widget("audio_numchannels")

        self.jack_latency_widget = widgets_tree.get_widget("jack_latency")
        self.jack_sampling_rate_widget = widgets_tree.get_widget("jack_sampling_rate")
        # system tab contents:
        self.network_admin_widget = widgets_tree.get_widget("network_admin")

        # MIDI tab
        self.midi_send_enabled_widget = widgets_tree.get_widget("midi_send_enabled")
        self.midi_recv_enabled_widget = widgets_tree.get_widget("midi_recv_enabled")
        self.midi_input_device_widget = widgets_tree.get_widget("midi_input_device")
        self.midi_output_device_widget = widgets_tree.get_widget("midi_output_device")
        self.midi_jitterbuffer_widget = widgets_tree.get_widget("midi_jitterbuffer")

        # switch to Kiosk mode if asked
        if self.kiosk_mode_on:
            self.main_window.set_decorated(False)
        else:
            # Removes the sytem_tab 
            tab_num = self.main_tabs_widget.page_num(self.system_tab_contents_widget)
            print "Removing tab number %d." % (tab_num)
            self.main_tabs_widget.remove_page(tab_num)
        
        self.is_fullscreen = False
        if fullscreen:
            print("Making the main window fullscreen.")
            self.toggle_fullscreen()
        
        # ------------------ contact list view
        self.selection = self.contact_list_widget.get_selection()
        self.selection.connect("changed", self.on_contact_list_changed, None) 
        self.contact_tree = gtk.ListStore(str)
        self.contact_list_widget.set_model(self.contact_tree)
        column = gtk.TreeViewColumn(_("Contacts"), gtk.CellRendererText(), markup=False)
        self.contact_list_widget.append_column(column)
        # TODO: those state variables interactive/not could be merged into a single one
        # preview:
        self.preview_manager = preview.Preview(self.app)
        self.video_preview_icon_widget = widgets_tree.get_widget("video_preview_icon")
        self.preview_manager.state_changed_signal.connect(self.on_preview_manager_state_changed)
        self.main_window.show()
        
        # recurring calls:
        self._streaming_state_check_task = task.LoopingCall(self.update_streaming_state)
        self._streaming_state_check_task.start(1.0, now=False)
        self._update_id_task = task.LoopingCall(self.update_local_ip)
        def _start_update_id():
            self._update_id_task.start(10.0, now=True)
        reactor.callLater(0, _start_update_id)
        # The main app must call init_widgets_value
   
    #TODO: for the preview in the drawing area   
    #def on_expose_event(self, widget, event):
    #    self.preview_xid = widget.window.xid
    #    return False

    # ------------------ window events and actions --------------------
    def toggle_fullscreen(self):
        """
        Toggles the fullscreen mode on/off.
        """
        if self.is_fullscreen:
            self.main_window.unfullscreen()
        else:
            self.main_window.fullscreen()

    def on_window_state_event(self, widget, event):
        """
        Called when toggled fullscreen.
        """
        self.is_fullscreen = event.new_window_state & gtk.gdk.WINDOW_STATE_FULLSCREEN != 0
        print('fullscreen %s' % (self.is_fullscreen))
        return True
    
    def on_main_window_deleted(self, *args):
        """
        Destroy method causes application to exit
        when main window closed
        """
        return self._confirm_and_quit()
        
    def _confirm_and_quit(self):
        def _cb(result):
            if result:
                print("Destroying the window.")
                self.main_window.destroy()
            else:
                print("Not quitting.")
        # If you return FALSE in the "delete_event" signal handler,
        # GTK will emit the "destroy" signal. Returning TRUE means
        # you don't want the window to be destroyed.
        # This is useful for popping up 'are you sure you want to quit?'
        # type dialogs. 
        if self.app.config.confirm_quit and self.app.has_session():
            d = dialogs.YesNoDialog.create(_("Really quit ?\nAll streaming processes will quit as well."), parent=self.main_window)
            d.addCallback(_cb)
            return True
        else:
            _cb(True)
            return False
    
    def on_main_window_destroyed(self, *args):
        # TODO: confirm dialog!
        if reactor.running:
            print("reactor.stop()")
            reactor.stop()

    # --------------- slots for some widget events ------------

    def on_preview_area_realize(self, *args):
        # avoid bad xid errors
        gtk.gdk.display_get_default().sync()
        xid = self.preview_area_widget.window.xid
        print("Preview area X Window ID: %s" % (xid))
        self.preview_area_x_window_id = xid
        self.preview_area_widget.window.set_background(gtk.gdk.Color(0, 0, 0)) # black

    def on_preview_manager_state_changed(self, manager, new_state):
        if new_state == process.STATE_STOPPED:
            print("Making the preview button to False since the preview process died.")
            if self.preview_manager.is_busy(): # very unlikely
                self.preview_manager.stop()
            self.video_preview_icon_widget.set_from_stock(gtk.STOCK_MEDIA_PLAY, 4)
            self._widgets_changed_by_user = False
            self.video_view_preview_widget.set_active(False)
            self._widgets_changed_by_user = True
        elif new_state == process.STATE_STARTING:
            self.video_preview_icon_widget.set_from_stock(gtk.STOCK_MEDIA_STOP, 4)

    def close_preview_if_running(self):
        """
        @rtype: L{Deferred}
        """
        def _cl(deferred):
            if self.preview_manager.is_busy():
                reactor.callLater(0.01, _cl, deferred)
            else:
                deferred.callback(None)
        if self.preview_manager.is_busy():
            self.preview_manager.stop()
            deferred = defer.Deferred()
            reactor.callLater(0.01, _cl, deferred)
            return deferred
        else:
            return defer.succeed(None)
        
    def on_video_view_preview_toggled(self, widget):
        """
        Shows a preview of the video input.
        """
        #TODO: create a new process protocol for the preview window
        #TODO: stop it when starting to stream, if running
        #TODO: stop it when button is toggled to false.
        # It can be the user that pushed the button, or it can be toggled by the software.
        print 'video_view_preview toggled', widget.get_active()
        if self._widgets_changed_by_user:
            if widget.get_active():
                self.app.save_configuration() #gathers and saves
                self.preview_manager.start()
            else:
                self.preview_manager.stop()

    def on_main_tabs_switch_page(self, widget, notebook_page, page_number):
        """
        Called when the user switches to a different page.
        Pages names are : 
         * contacts_tab_contents
         * video_tab_contents
         * audio_tab_contents
         * system_tab_contents
         * about_tab_contents
        """
        tab_widget = widget.get_nth_page(page_number)
        tab_name = tab_widget.get_name()
        if tab_name == "contacts_tab_contents":
            self.invite_contact_widget.grab_default()
        elif tab_name == "video_tab_contents":
            self.app.poll_x11_devices()
            self.app.poll_camera_devices()
        elif tab_name == "audio_tab_contents":
            pass
        elif tab_name == "system_tab_contents":
            self.network_admin_widget.grab_default()
        elif tab_name == "midi_tab_contents":
            self.app.poll_midi_devices()

    def on_contact_list_changed(self, *args):
        # FIXME: what is args?
        tree_list, self.selected_contact_row = args[0].get_selected()
        if self.selected_contact_row:
            # make the edit, remove, invite buttons sensitive:
            self.edit_contact_widget.set_sensitive(True)
            self.remove_contact_widget.set_sensitive(True)
            self.invite_contact_widget.set_sensitive(True)
            # get selected contact
            self.selected_contact_index = tree_list.get_path(self.selected_contact_row)[0] # FIXME: this var should be deprecated
            self.app.address_book.selected_contact = self.app.address_book.contact_list[self.selected_contact_index] # FIXME: deprecate this!
            self.app.address_book.selected = self.selected_contact_index
            self.update_invite_button_with_contact_name()
        else:
            # make the edit, remove, invite buttons sensitive:
            self.edit_contact_widget.set_sensitive(False)
            self.remove_contact_widget.set_sensitive(False)
            self.invite_contact_widget.set_sensitive(False)
            # no contact is selected
            self.app.address_book.selected_contact = None

    # ---------------------- slots for addressbook widgets events --------
    
    def on_contact_double_clicked(self, *args):
        """
        When a contact in the list is double-clicked, 
        shows the edit contact dialog.
        """
        self.on_edit_contact_clicked(args)

    def on_add_contact_clicked(self, *args):
        """
        Pops up a dialog to be filled with new contact infos.
        
        The add_contact buttons has been clicked.
        """
        self.app.address_book.current_contact_is_new = True
        # Update the text in the edit/new contact dialog:
        self.contact_name_widget.set_text("")
        self.contact_addr_widget.set_text("")
        self.contact_auto_accept_widget.set_active(False)
        self.edit_contact_window.show()

    def on_remove_contact_clicked(self, *args):
        """
        Upon confirmation, the selected contact is removed.
        """
        def _on_confirm_result(result):
            if result:
                del self.app.address_book.contact_list[self.selected_contact_index]
                self.contact_tree.remove(self.selected_contact_row)
                num = self.selected_contact_index - 1
                if num < 0:
                    num = 0
                self.selection.select_path(num)
        text = _("<b><big>Delete this contact from the list?</big></b>\n\nAre you sure you want "
            "to delete this contact from the list?")
        self.show_confirm_dialog(text, _on_confirm_result)

    def on_edit_contact_clicked(self, *args):
        """
        Shows the edit contact dialog.
        """
        contact = self.app.address_book.selected_contact
        self.contact_name_widget.set_text(contact["name"])
        self.contact_addr_widget.set_text(contact["address"])
        auto_accept = False
        if contact["auto_accept"]:
            auto_accept = True
            print('auto accept should be true')
        self.contact_auto_accept_widget.set_active(auto_accept)
        self.edit_contact_window.show() # addr

    def on_edit_contact_cancel_clicked(self, *args):
        """
        The cancel button in the "edit_contact" window has been clicked.
        Hides the edit_contact window.
        """
        self.edit_contact_window.hide()

    def on_edit_contact_save_clicked(self, *args):
        """
        The save button in the "edit_contact" window has been clicked.
        Hides the edit_contact window and saves the changes. (new or modified contact)
        """
        def _when_valid_save():
            # Saves contact info after it's been validated and then closes the window
            # THIS IS WHERE WE CREATE THE CONTACTS IN THE ADDRESSBOOK
            # TODO: move to a dedicated function in save.py or so.
            contact = {
                "name": self.contact_name_widget.get_text(),
                "address": addr,
                "auto_accept": self.contact_auto_accept_widget.get_active(),
                }
            contact_markup = format_contact_markup(contact)
            if self.app.address_book.current_contact_is_new:
                self.contact_tree.append([contact_markup]) # add it to the tree list
                self.app.address_book.contact_list.append(contact) # and the internal address book
                self.selection.select_path(len(self.app.address_book.contact_list) - 1) # select it ...?
                self.app.address_book.selected_contact = self.app.address_book.contact_list[len(self.app.address_book.contact_list) - 1] #FIXME: we should not copy a dict like that
                self.app.address_book.current_contact_is_new = False # FIXME: what does that mean?
            else:
                self.contact_tree.set_value(self.selected_contact_row, 0, contact_markup)
                self.app.address_book.contact_list[self.selected_contact_index] = contact # FIXME: this is flaky. make some functions to handle this
            self.app.address_book.selected_contact = contact
            self.edit_contact_window.hide()

        # Validate the address
        addr = self.contact_addr_widget.get_text()
        if not network.validate_address(addr):
            dialogs.ErrorDialog.create(_("The address is not valid\n\nEnter a valid address\nExample: 192.0.32.10 or example.org"), parent=self.main_window)
            return
        # save it.
        _when_valid_save()

    # ---------------------------- Custom system tab buttons ---------------

    def on_network_admin_clicked(self, *args):
        """
        Opens the network-admin Gnome applet.
        """
        process.run_once("gksudo", "network-admin")

    def on_system_shutdown_clicked(self, *args):
        """
        Shuts down the computer.
        """
        def _on_confirm_result(result):
            if result:
                process.run_once("gksudo", "shutdown -h now")

        text = _("<b><big>Shutdown the computer?</big></b>\n\nAre you sure you want to shutdown the computer now?")
        self.show_confirm_dialog(text, _on_confirm_result)

    def on_system_reboot_clicked(self, *args):
        """
        Reboots the computer.
        """
        def _on_confirm_result(result):
            if result:
                process.run_once("gksudo", "shutdown -r now")

        text = _("<b><big>Reboot the computer?</big></b>\n\nAre you sure you want to reboot the computer now?")
        self.show_confirm_dialog(text, _on_confirm_result)

    def on_maintenance_apt_update_clicked(self, *args):
        """
        Opens APT update manager.
        """
        process.run_once("gksudo", "update-manager")

    def on_maintenance_send_info_clicked(self, *args):
        """
        Sends an email to SAT with this information : 
         * milhouse version
         * kernel version
         * Loaded kernel modules
        """
        # TODO: move this to an other file.
        def _on_confirm_result(result):
            milhouse_version = "unknown"
            if result:
                msg = "--- milhouse_version ---\n" + milhouse_version + "\n"
                msg += "--- uname -a ---\n"
                try:
                    w, r, err = os.popen3('uname -a')
                    msg += r.read() + "\n"
                    errRead = err.read()
                    if errRead:
                        msg += errRead + "\n"
                    w.close()
                    r.close()
                    err.close()
                except:
                    msg += "Error executing 'uname -a'\n"
                msg += "--- lsmod ---\n"
                try:
                    w, r, err = os.popen3('lsmod')
                    msg += r.read()
                    errRead = err.read()
                    if errRead:
                        msg += "\n" + errRead
                    w.close()
                    r.close()
                    err.close()
                except:
                    msg += "Error executing 'lsmod'"
                fromaddr = self.app.config.email_info
                toaddrs  = self.app.config.email_info
                toaddrs = toaddrs.split(', ')
                server = smtplib.SMTP(self.app.config.smtpserver)
                server.set_debuglevel(0)
                try:
                    server.sendmail(fromaddr, toaddrs, msg)
                except:
                    dialogs.ErrorDialog.create(_("Could not send info.\nCheck your internet connection."), parent=self.main_window)
                server.quit()
        
        text = _("<b><big>Send the settings?</big></b>\n\nAre you sure you want to send your computer settings to the administrator of scenic?")
        self.show_confirm_dialog(text, _on_confirm_result)

    # --------------------- configuration and widgets value ------------

    def _gather_configuration(self):
        """
        Updates the configuration with the value of each widget.
        """
        print("Gathering configuration from the GUI widgets.")
        # VIDEO SIZE:
        video_capture_size = _get_combobox_value(self.video_capture_size_widget)
        self.app.config.video_capture_size = video_capture_size
        print ' * video_capture_size:', self.app.config.video_capture_size
        # DISPLAY:
        video_display = _get_combobox_value(self.video_display_widget)
        self.app.config.video_display = video_display
        print ' * video_display:', self.app.config.video_display
        # VIDEO SOURCE AND DEVICE:
        video_source = _get_combobox_value(self.video_source_widget)
        if video_source == "Color bars":
            self.app.config.video_source = "videotestsrc"
        else:
            #video_device = self.app.parse_v4l2_device_name(video_source)
            #if video_device is None:
            #    print "Could not find video device %s" % (video_source)
            #elif video_source.startswith("/dev/video"): # TODO: firewire!
            #TODO: check if it is a v4l2 device.
            self.app.config.video_source = "v4l2src"
            self.app.config.video_device = video_source # Using the name and id as a video_device
        print ' * videosource:', self.app.config.video_source
        # VIDEO CODEC:
        video_codec = _get_combobox_value(self.video_codec_widget)
        self.app.config.video_codec = VIDEO_CODECS[video_codec]
        print ' * video_codec:', self.app.config.video_codec
        # VIDEO ASPECT RATIO:
        video_aspect_ratio = _get_combobox_value(self.aspect_ratio_widget)
        self.app.config.video_aspect_ratio = video_aspect_ratio
        print ' * video_aspect_ratio:', self.app.config.video_aspect_ratio
        #VIDEO FULLSCREEN
        video_fullscreen = self.video_fullscreen_widget.get_active()
        self.app.config.video_fullscreen = video_fullscreen
        print ' * video_fullscreen:', self.app.config.video_fullscreen
        #VIDEO DEINTERLACE
        video_deinterlace = self.video_deinterlace_widget.get_active()
        self.app.config.video_deinterlace = video_deinterlace
        print ' * video_deinterlace:', self.app.config.video_deinterlace
        # VIDEO JITTERBUFFER
        video_jitterbuffer = self.video_jitterbuffer_widget.get_value_as_int() # spinbutton
        self.app.config.video_jitterbuffer = video_jitterbuffer
        print ' * video_jitterbuffer:', self.app.config.video_jitterbuffer
        # VIDEO BITRATE
        video_bitrate = self.video_bitrate_widget.get_value() # spinbutton (float)
        self.app.config.video_bitrate = float(video_bitrate)
        print ' * video_bitrate:', self.app.config.video_bitrate
        # VIDEO PREVIEW
        preview_in_window = self.preview_in_window_widget.get_active()
        self.app.config.preview_in_window = preview_in_window
        print " * preview_in_window: ", preview_in_window
        
        # AUDIO:
        audio_source_readable = _get_combobox_value(self.audio_source_widget)
        audio_codec_readable = _get_combobox_value(self.audio_codec_widget)
        audio_numchannels = self.audio_numchannels_widget.get_value_as_int() # spinbutton
        print " * audio_source:", audio_source_readable
        print " * audio_codec:", audio_codec_readable
        print " * audio_numchannels:", audio_numchannels
        self.app.config.audio_source = AUDIO_SOURCES[audio_source_readable]
        self.app.config.audio_codec = AUDIO_CODECS[audio_codec_readable]
        # FIXME: the interface should already prevent this case from happening
        if audio_numchannels > 2 and self.app.config.audio_codec == "mp3":
            print("Will receive 2 channels, since the MP3 codec allows a maximum of 2 channels.")
            print("This should have been prevented by the widgets logic iteself. Not likely to occur.")
            dialogs.ErrorDialog.create(_("Will receive 2 channels, since the MP3 codec allows a maximum of 2 channels."))
            audio_numchannels = 2
        self.app.config.audio_channels = audio_numchannels
        print " * audio_numchannels", self.app.config.audio_channels
        
        # MIDI:
        midi_send_enabled = self.midi_send_enabled_widget.get_active()
        midi_recv_enabled = self.midi_recv_enabled_widget.get_active()
        midi_input = _get_combobox_value(self.midi_input_device_widget)
        midi_output = _get_combobox_value(self.midi_output_device_widget)
        midi_jitterbuffer = self.midi_jitterbuffer_widget.get_value_as_int() 
        print " * midi_send_enabled:", midi_send_enabled
        print " * midi_recv_enabled:", midi_recv_enabled
        print " * midi_input_device:", midi_input
        print " * midi_output_device:", midi_output
        print " * midi_jitterbuffer:", midi_jitterbuffer
        self.app.config.midi_send_enabled = midi_send_enabled
        self.app.config.midi_recv_enabled = midi_recv_enabled
        self.app.config.midi_input_device = midi_input
        self.app.config.midi_output_device = midi_output
        self.app.config.midi_jitterbuffer = midi_jitterbuffer

    def update_widgets_with_saved_config(self):
        """
        Called once at startup.
         * Once the config file is read, and the devices have been polled
         * Sets the value of each widget according to the data stored in the configuration file.
        It could be called again, once another config file has been read.
        """
        self._widgets_changed_by_user = False
        print("Changing widgets value according to configuration.")
        print(self.app.config.__dict__)
        # VIDEO CAPTURE SIZE:
        video_capture_size = self.app.config.video_capture_size
        _set_combobox_choices(self.video_capture_size_widget, ALL_SUPPORTED_SIZE)
        _set_combobox_value(self.video_capture_size_widget, video_capture_size)
        print ' * video_capture_size:', video_capture_size
        # DISPLAY:
        video_display = self.app.config.video_display
        _set_combobox_value(self.video_display_widget, video_display)
        print ' * video_display:', video_display
        # VIDEO SOURCE AND DEVICE:
        if self.app.config.video_source == "videotestsrc":
            video_source = "Color bars"
        elif self.app.config.video_source == "v4l2src":
            video_source = self.app.config.video_device
        _set_combobox_value(self.video_source_widget, video_source)
        print ' * videosource:', video_source
        # VIDEO CODEC:
        video_codec = _get_key_for_value(VIDEO_CODECS, self.app.config.video_codec)
        _set_combobox_value(self.video_codec_widget, video_codec)
        print ' * video_codec:', video_codec
        # VIDEO ASPECT RATIO:
        video_aspect_ratio = self.app.config.video_aspect_ratio
        _set_combobox_value(self.aspect_ratio_widget, video_aspect_ratio)
        print ' * video_aspect_ratio:', video_aspect_ratio
        # VIDEO FULLSCREEN:
        video_fullscreen = self.app.config.video_fullscreen
        self.video_fullscreen_widget.set_active(video_fullscreen)
        print ' * video_fullscreen:', video_fullscreen
        # VIDEO DEINTERLACE:
        video_deinterlace = self.app.config.video_deinterlace
        self.video_deinterlace_widget.set_active(video_deinterlace)
        print ' * video_deinterlace:', video_deinterlace
        # VIDEO JITTERBUFFER
        video_jitterbuffer = self.app.config.video_jitterbuffer
        self.video_jitterbuffer_widget.set_value(video_jitterbuffer) # spinbutton
        print ' * video_jitterbuffer:', video_jitterbuffer
        # VIDEO BITRATE
        video_bitrate = self.app.config.video_bitrate
        self.video_bitrate_widget.set_value(video_bitrate) # spinbutton
        print ' * video_bitrate:', video_bitrate
        # VIDEO PREVIEW
        preview_in_window = self.app.config.preview_in_window
        self.preview_in_window_widget.set_active(preview_in_window)
        print " * preview_in_window: ", preview_in_window
        
        # ADDRESSBOOK:
        # Init addressbook contact list:
        self.app.address_book.selected_contact = None
        self.app.address_book.current_contact_is_new = False
        if len(self.app.address_book.contact_list) > 0:
            for contact in self.app.address_book.contact_list:
                contact_markup = format_contact_markup(contact)
                self.contact_tree.append([contact_markup])
            self.selection.select_path(self.app.address_book.selected)
        else:
            self.edit_contact_widget.set_sensitive(False)
            self.remove_contact_widget.set_sensitive(False)
            self.invite_contact_widget.set_sensitive(False)
        # change invite button with the name of the selected contact
        self.update_invite_button_with_contact_name()
        # AUDIO:
        audio_source_readable = _get_key_for_value(AUDIO_SOURCES, self.app.config.audio_source)
        audio_codec = _get_key_for_value(AUDIO_CODECS, self.app.config.audio_codec)
        audio_numchannels = self.app.config.audio_channels
        print " * audio_source:", audio_source_readable
        print " * audio_codec:", audio_codec
        print " * audio_numchannels:", audio_numchannels
        self.audio_numchannels_widget.set_value(audio_numchannels) # spinbutton
        _set_combobox_value(self.audio_source_widget, audio_source_readable)
        _set_combobox_value(self.audio_codec_widget, audio_codec)
        
        # MIDI:
        print "MIDI send enabled:", self.app.config.midi_send_enabled
        print "MIDI recv enabled:", self.app.config.midi_recv_enabled
        print "MIDI input:", self.app.config.midi_input_device
        print "MIDI output:", self.app.config.midi_output_device
        print "MIDI jitterbuffer:", self.app.config.midi_jitterbuffer
        self.midi_send_enabled_widget.set_active(self.app.config.midi_send_enabled)
        self.midi_recv_enabled_widget.set_active(self.app.config.midi_recv_enabled)
        _set_combobox_value(self.midi_input_device_widget, self.app.config.midi_input_device)
        _set_combobox_value(self.midi_output_device_widget, self.app.config.midi_output_device)
        self.make_midi_widget_sensitive_or_not()
        self.midi_jitterbuffer_widget.set_value(self.app.config.midi_jitterbuffer)
        self._widgets_changed_by_user = True

    def update_streaming_state(self):
        """
        Changes the sensitivity and state of many widgets according to if we are streaming or not.
        
        Makes most of the audio/video buttons and widgets sensitive or not.
        Changes the invite button:
         * the icon
         * the label
        Makes the contact list sensitive or not.
        """
        self._toggle_streaming_state_sensitivity()
        self._update_rtcp_stats()

    def _toggle_streaming_state_sensitivity(self):
        _video_widgets_to_toggle_sensitivity = [
            self.video_capture_size_widget,
            self.video_source_widget,
            self.aspect_ratio_widget,
            self.video_view_preview_widget,
            self.preview_in_window_widget, 
            ]
        
        _other_widgets_to_toggle_sensitivity = [
            self.audio_source_widget,
            self.audio_codec_widget,
            self.audio_numchannels_widget,
            self.contact_list_widget,
            self.add_contact_widget,
            self.remove_contact_widget,
            self.edit_contact_widget,
            self.video_fullscreen_widget,
            self.video_deinterlace_widget,
            self.video_jitterbuffer_widget,
            self.video_codec_widget,
            self.video_display_widget,
            self.midi_input_device_widget, 
            self.midi_output_device_widget,
            self.midi_send_enabled_widget, 
            self.midi_recv_enabled_widget,
            self.midi_jitterbuffer_widget,
            ]
        
        self.update_bitrate_and_codec()
        
        is_streaming = self.app.has_session()
        is_previewing =  self.preview_manager.is_busy()
        if is_streaming:
            details = self.app.streamer_manager.session_details
        _contact_list_currently_sensitive = self.contact_list_widget.get_property("sensitive")
        streaming_state_has_changed = is_streaming == _contact_list_currently_sensitive
        if streaming_state_has_changed:
            print("streaming state has changed to %s" % (is_streaming))
            if is_streaming:
                text = _("Stop streaming")
                self.invite_label_widget.set_text(text)
                icon = gtk.STOCK_CONNECT
            else:
                self.update_invite_button_with_contact_name()
                icon = gtk.STOCK_DISCONNECT
            self.invite_icon_widget.set_from_stock(icon, 4)
            
            # Toggle sensitivity of many widgets:
            new_sensitivity = not is_streaming
            print 'Got to change the sensitivity of many widgets to', new_sensitivity
            for widget in _other_widgets_to_toggle_sensitivity:
                widget.set_sensitive(new_sensitivity)
            for widget in _video_widgets_to_toggle_sensitivity:
                widget.set_sensitive(new_sensitivity)
                
            # Update the summary: 
            # peer: --------------------------------
            if is_streaming:
                peer_name = details["peer"]["name"]
                if details["peer"]["name"] != details["peer"]["address"]:
                    peer_name += " (%s)" % (details["peer"]["address"])
                self.info_peer_widget.set_text(peer_name)
            else:
                self.info_peer_widget.set_text(_("Not connected"))
            
            self.make_midi_widget_sensitive_or_not()       
        
        # also clean up the preview drawing area every second
        if self.preview_manager.is_busy():
            if self.preview_in_window_widget.get_property('sensitive'): # check if we have to change their state.
                for widget in _video_widgets_to_toggle_sensitivity:
                    if widget is not self.video_view_preview_widget:
                        widget.set_sensitive(False)
                # change icon
        else:
            if not is_streaming: # FIXME
                #if self.video_preview_icon_widget.get_stock() == gtk.STOCK_MEDIA_STOP:
                for widget in _video_widgets_to_toggle_sensitivity:
                    widget.set_sensitive(True)
            if self.preview_area_x_window_id is not None:
                if self.preview_area_widget.window is not None:
                    self.preview_area_widget.window.clear()

    def update_invite_button_with_contact_name(self):
        contact = self.app.address_book.selected_contact
        if contact is None:
            text = _("Please select a contact")
        else:
            text = _("Invite %(contact)s") % {"contact": contact["name"]}
        self.invite_label_widget.set_text(text)

    def make_midi_widget_sensitive_or_not(self):
        # make the MIDI widget insensitive if disabled
        print("make_midi_widget_sensitive_or_not")
        is_streaming = self.app.has_session()
        if not is_streaming:
            if not self.app.config.midi_send_enabled:
                self.midi_input_device_widget.set_sensitive(False)
            else:
                self.midi_input_device_widget.set_sensitive(True)
                
            if not self.app.config.midi_recv_enabled:
                self.midi_output_device_widget.set_sensitive(False)
                self.midi_jitterbuffer_widget.set_sensitive(False)
            else:
                self.midi_output_device_widget.set_sensitive(True)
                self.midi_jitterbuffer_widget.set_sensitive(True)

    def on_midi_send_enabled_toggled(self, *args):
        if self._widgets_changed_by_user:
            self._gather_configuration()
            self.make_midi_widget_sensitive_or_not()
    
    def on_midi_recv_enabled_toggled(self, *args):
        if self._widgets_changed_by_user:
            self._gather_configuration()
            self.make_midi_widget_sensitive_or_not()

    def _update_rtcp_stats(self):
        is_streaming = self.app.has_session()
        # update the audio and video summary:(even if the state has not just changed)
        if is_streaming:
            def _format_bitrate(bitrate):
                """ Returns formatted bitrate string """
                BITS_PER_MBIT = 1000000.0
                BITS_PER_KBIT = 1000.0
                if bitrate is not None:
                    divisor = BITS_PER_MBIT
                    prefix = "M"
                    if bitrate < BITS_PER_MBIT:
                        divisor = BITS_PER_KBIT
                        prefix = "K"
                    return " " + _("%2.2f %sbits/s") % (bitrate / divisor, prefix)
                else:
                    return ""

            details = self.app.streamer_manager.session_details
            rtcp_stats = self.app.streamer_manager.rtcp_stats
            # send video: --------------------------------
            _info_send_video = _("%(width)dx%(height)d %(codec)s") % {
                "width": details["send"]["video"]["width"], 
                "height": details["send"]["video"]["height"], 
                "codec": details["send"]["video"]["codec"], 
                }
            _info_send_video += _format_bitrate(rtcp_stats["send"]["video"]["bitrate"])
            _info_send_video += "\n"
            #_video_packetloss = rtcp_stats["send"]["video"]["packets-loss-percent"]
            _info_send_video += _("Jitter: %(jitter)d ns") % {# % is escaped with an other %
                "jitter": rtcp_stats["send"]["video"]["jitter"]
                }
            #_info_send_video += _("jitter: %(jitter)d ns. packet loss: %(packetloss)2.2f%%.") % {# % is escaped with an other %
            #    "jitter": rtcp_stats["send"]["video"]["jitter"],
            #    "packetloss": _video_packetloss
            #    }
            #print("info send video: " + _info_send_video)
            self.info_send_video_widget.set_text(_info_send_video)
            # send audio: --------------------------------
            _info_send_audio = _("%(numchannels)d-channel %(codec)s") % {
                "numchannels": details["send"]["audio"]["numchannels"], 
                "codec": details["send"]["audio"]["codec"] 
                }
            _info_send_audio += _format_bitrate(rtcp_stats["send"]["audio"]["bitrate"])
            _info_send_audio += "\n"
            #_audio_packetloss = rtcp_stats["send"]["audio"]["packets-loss-percent"]
            _info_send_audio += _("Jitter: %(jitter)d ns") % { # % is escaped with an other %
                "jitter": rtcp_stats["send"]["audio"]["jitter"]
                }
            #print("info send audio: " + _info_send_audio)
            self.info_send_audio_widget.set_text(_info_send_audio)
            # recv video: --------------------------------
            _info_recv_video = _("%(width)dx%(height)d %(codec)s") % {
                "width": details["receive"]["video"]["width"], 
                "height": details["receive"]["video"]["height"], 
                "codec": details["receive"]["video"]["codec"], 
                }
            _info_recv_video += _format_bitrate(rtcp_stats["receive"]["video"]["bitrate"])
            _info_recv_video += "\n" + _("Display: %(display)s") % {"display": details["receive"]["video"]["display"]}
            if details["receive"]["video"]["fullscreen"]:
                _info_recv_video += "\n" + _("Fullscreen is enabled.")

            #print("info recv video: " + _info_recv_video)
            self.info_receive_video_widget.set_text(_info_recv_video)
            # recv audio: --------------------------------
            _info_recv_audio = _("%(numchannels)d-channel %(codec)s") % {
                "numchannels": details["receive"]["audio"]["numchannels"], 
                "codec": details["receive"]["audio"]["codec"] 
                }
            _info_recv_audio += _format_bitrate(rtcp_stats["receive"]["audio"]["bitrate"])
            self.info_receive_audio_widget.set_text(_info_recv_audio)
            # MIDI : --------------------------
            _info_recv_midi = ""
            _info_send_midi = ""
            if details["receive"]["midi"]["enabled"]:
                _info_recv_midi += _("Receiving MIDI") + "\n"
                #_info_recv_midi += _("Output device: %(name)s" % {"name": self.app.config.midi_output_device}) + "\n"
                _info_recv_midi += _("Jitter buffer: %(jitterbuffer)d ms" % {"jitterbuffer": self.app.config.midi_jitterbuffer})
            else:
                _info_recv_midi += _("Disabled")
            if details["send"]["midi"]["enabled"]:
                _info_send_midi += _("Sending MIDI") + "\n"
                #_info_send_midi += _("Input device: %(name)s" % {"name": self.app.config.midi_input_device})
            else:
                _info_send_midi += _("Disabled")
                
            self.info_receive_midi_widget.set_text(_info_recv_midi)
            self.info_send_midi_widget.set_text(_info_send_midi)
        else:
            self.info_send_video_widget.set_text("")
            self.info_send_audio_widget.set_text("")
            self.info_receive_video_widget.set_text("")
            self.info_receive_audio_widget.set_text("")
            self.info_receive_midi_widget.set_text("")
            self.info_send_midi_widget.set_text("")
        
    def update_local_ip(self):
        """
        Updates the local IP addresses widgets.
        Called every n seconds.
        """
        def _cb(result):
            """
            @param result: list of ipv4 addresses
            """
            num = len(result)
            txt = ""
            for i in range(len(result)):
                ip = result[i]
                txt += ip
                if i != num - 1:
                    txt += "\n"
            self.info_ip_widget.set_text(txt)
        deferred = networkinterfaces.list_network_interfaces_addresses()
        deferred.addCallback(_cb)
        
    def on_audio_codec_changed(self, widget):
        """
        Called when the user selects a different audio codec, updates
        the range of the numchannels box.
        """
        old_numchannels = self.audio_numchannels_widget.get_value()
        max_channels = None
        if _get_combobox_value(self.audio_codec_widget) == "MP3":
            max_channels = 2
        elif _get_combobox_value(self.audio_codec_widget) == "Raw":
            max_channels = 8
        elif _get_combobox_value(self.audio_codec_widget) == "Vorbis":
            max_channels = 24 
        # update range and clamp numchannels to new range 
        self.audio_numchannels_widget.set_range(1, max_channels)
        self.audio_numchannels_widget.set_value(min(old_numchannels, max_channels)) 

    def update_bitrate_and_codec(self):
        old_bitrate = self.video_bitrate_widget.get_value()
        codec = _get_combobox_value(self.video_codec_widget)
        is_streaming = self.app.has_session()
        if codec in VIDEO_BITRATE_MIN_MAX.keys():
            if is_streaming:
                self.video_bitrate_widget.set_sensitive(False)
            else:
                self.video_bitrate_widget.set_sensitive(True)
            mini = VIDEO_BITRATE_MIN_MAX[codec][0]
            maxi = VIDEO_BITRATE_MIN_MAX[codec][1]
            self.video_bitrate_widget.set_range(mini, maxi)
            self.video_bitrate_widget.set_value(min(maxi, max(old_bitrate, mini)))
        else:
            self.video_bitrate_widget.set_sensitive(False)
    
    def on_video_codec_changed(self, widget):
        self.update_bitrate_and_codec()
        
    def update_x11_devices(self):
        """
        Called once Application.poll_x11_devices has been run
        """
        x11_displays = [display["name"] for display in self.app.devices["x11_displays"]]
        print("Updating X11 displays with values %s" % (x11_displays))
        _set_combobox_choices(self.video_display_widget, x11_displays)

    def update_midi_devices(self):
        """
        Called once Application.poll_midi_devices has been run
        """
        self._widgets_changed_by_user = False
        input_devices = [self.app.format_midi_device_name(device) for device in self.app.devices["midi_input_devices"]]
        output_devices = [self.app.format_midi_device_name(device) for device in self.app.devices["midi_output_devices"]]
        print("Updating MIDI devices with values %s %s" % (input_devices, output_devices))
        _set_combobox_choices(self.midi_input_device_widget, input_devices)
        _set_combobox_choices(self.midi_output_device_widget, output_devices)
        self._widgets_changed_by_user = True

    def update_camera_devices(self):
        """
        Called once Application.poll_camera_devices has been run
        """
        self._widgets_changed_by_user = False
        video_sources = [self.app.format_v4l2_device_name(dev) for dev in self.app.devices["cameras"].values()]
        video_sources.insert(0, VIDEO_TEST_INPUT)
        print("Updating video sources with values %s" % (video_sources))
        _set_combobox_choices(self.video_source_widget, video_sources)
        self.update_v4l2_inputs_size_and_norm()
        self._widgets_changed_by_user = True

    def update_v4l2_inputs_size_and_norm(self):
        """
        Called when : 
         * user chooses a different video source.
        If the selected is not a V4L2, disables the input and norm widgets.
        """
        value = _get_combobox_value(self.video_source_widget)
        self._widgets_changed_by_user = False
        # change choices and value:
        if value == VIDEO_TEST_INPUT:
            # INPUTS:
            self.v4l2_input_widget.set_sensitive(False)
            self.v4l2_input_widget.set_active(-1)
            # STANDARD:
            self.v4l2_standard_widget.set_sensitive(False)
            self.v4l2_standard_widget.set_active(-1)
            # SIZE:
            _set_combobox_choices(self.video_capture_size_widget, ALL_SUPPORTED_SIZE)
        else:
            # INPUTS:
            current_camera_name = _get_combobox_value(self.video_source_widget)
            cam = self.app.parse_v4l2_device_name(current_camera_name)
            if cam is None:
                print "v4l2 device is none !!", current_camera_name
                # INPUTS:
                self.v4l2_input_widget.set_sensitive(False)
                self.v4l2_input_widget.set_active(-1)
                # STANDARD:
                self.v4l2_standard_widget.set_sensitive(False)
                self.v4l2_standard_widget.set_active(-1)
                # SIZE:
                _set_combobox_choices(self.video_capture_size_widget, ALL_SUPPORTED_SIZE)
            else:
                current_input = cam["input"]
                if current_input is not None: # check if device has many inputs
                    self.v4l2_input_widget.set_sensitive(True)
                    _set_combobox_choices(self.v4l2_input_widget, cam["inputs"])
                    _set_combobox_value(self.v4l2_input_widget, cam["inputs"][current_input]) # which in turn calls on_v4l2_input_changed
                else:
                    self.v4l2_input_widget.set_sensitive(False)
                    self.v4l2_input_widget.set_active(-1)
                
                # STANDARD: 
                current_standard = cam["standard"]
                if current_standard is not None: # check if device supports different standards
                    self.v4l2_standard_widget.set_sensitive(True)
                    _set_combobox_choices(self.v4l2_standard_widget, VIDEO_STANDARDS)
                    _set_combobox_value(self.v4l2_standard_widget, cam["standard"]) # which in turn calls on_v4l2_standard_changed
                else:
                    self.v4l2_standard_widget.set_sensitive(False)
                    self.v4l2_standard_widget.set_active(-1)
                #self.v4l2_standard_widget.set_sensitive(True)
                # SIZE:
                print "supported sizes: ", cam["supported_sizes"]
                _set_combobox_choices(self.video_capture_size_widget, cam["supported_sizes"]) # TODO: more test sizes
        # once done:
        self._widgets_changed_by_user = True
            
    def on_video_source_changed(self, widget):
        """
        Called when the user changes the video source.
         * updates the input
        """
        if self._widgets_changed_by_user: 
            full_name = _get_combobox_value(self.video_source_widget)
            if full_name != VIDEO_TEST_INPUT:
                dev = self.app.parse_v4l2_device_name(full_name)
                current_camera_name = dev["name"]
                self.app.poll_camera_devices()
            self.update_v4l2_inputs_size_and_norm()

    def on_v4l2_standard_changed(self, widget):
        """
        When the user changes the V4L2 standard, we actually change this standard using milhouse.
        Calls `milhouse --videodevice /dev/videoX --v4l2-standard XXX
        Values are either NTSC or PAL.
        """
        if self._widgets_changed_by_user: 
            # change standard for device
            full_name = _get_combobox_value(self.video_source_widget)
            if full_name != VIDEO_TEST_INPUT:
                dev = self.app.parse_v4l2_device_name(full_name)
                current_camera_name = dev["name"]
                def _cb2(result):
                    # callback for the poll_cameras_devices deferred.
                    # check if successfully changed norm
                    # see below
                    cameras = self.app.devices["cameras"]
                    try:
                        cam = cameras[current_camera_name]
                    except KeyError, e:
                        print("Camera %s disappeared !" % (current_camera_name))
                    else:
                        actual_standard = cam["standard"]
                        if actual_standard != standard_name:
                            msg = _("Could not change V4L2 standard from %(current_standard)s to %(desired_standard)s for device %(device_name)s.") % {"current_standard": actual_standard, "desired_standard": standard_name, "device_name": current_camera_name}
                            print(msg)
                            dialogs.ErrorDialog.create(msg, parent=self.main_window)
                            
                            self._widgets_changed_by_user = False
                            _set_combobox_value(self.v4l2_standard_widget, actual_standard)
                            self._widgets_changed_by_user = True
                            # Maybe we should show an error dialog in that case, or set the value to what it really is.
                        else:
                            print("Successfully changed standard to %s for device %s." % (actual_standard, current_camera_name))
                            print("Now polling cameras.")
                    self.v4l2_standard_widget.set_sensitive(True)
                
                standard_name = _get_combobox_value(widget)
                #cam = self.app.devices["cameras"][current_camera_name]
                self.v4l2_standard_widget.set_sensitive(False)
                d = cameras.set_v4l2_video_standard(device_name=current_camera_name, standard=standard_name)
                def _cb(result):
                    d2 = self.app.poll_camera_devices()
                    d2.addCallback(_cb2)
                    
                d.addCallback(_cb)
        
    def on_v4l2_input_changed(self, widget):
        """
        When the user changes the V4L2 input, we actually change this input using milhouse.
        Calls `milhouse --videodevice /dev/videoX --v4l2-input N
        """
        def _cb2(cameras):
            try:
                cam = cameras[current_camera_name]
            except KeyError, e:
                print("Camera %s disappeared !" % (current_camera_name))
            else:
                actual_input = cam["input"]
                if actual_input != input_number:
                    msg = _("Could not change V4L2 input from %(current_input)s to %(desired_input)s for device %(device_name)s.") % {"current_input": actual_input, "desired_input": input_number, "device_name": current_camera_name}
                    print(msg)
                    # Maybe we should show an error dialog in that case, or set the value to what it really is.
                else:
                    print("Successfully changed input to %s for device %s." % (actual_input, current_camera_name))

        def _cb(result):
            d2 = cameras.list_cameras()
            d2.addCallback(_cb2)

        if self._widgets_changed_by_user:
            # change input for device
            current_camera_name = _get_combobox_value(self.video_source_widget)
            if current_camera_name != VIDEO_TEST_INPUT:
                input_name = _get_combobox_value(widget) # self.v4l2_input_widget
                #cam = self.app.devices["cameras"][input_name]
                cam = self.app.parse_v4l2_device_name(current_camera_name)
                if cam is None:
                    print("No such v4l2 device: %s" % (input_name))
                else:
                    input_number = cam["inputs"].index(input_name)
                    d = cameras.set_v4l2_input_number(device_name=cam["name"], input_number=input_number)
                    d.addCallback(_cb)

    # -------------------------- menu items -----------------
    
    def on_about_menu_item_activate(self, menu_item):
        About.create() # TODO: set parent window ?
    
    def on_quit_menu_item_activated(self, menu_item):
        """
        Quits the application.
        """
        print menu_item, "chosen"
        self._confirm_and_quit()
    
    def on_help_menu_item_activated(self, menu_item):
        """
        Opens a web browser to the scenic web site.
        """
        print menu_item, "chosen"
        url = ONLINE_HELP_URL 
        webbrowser.open(url)

    # ---------------------- invitation dialogs -------------------

    def on_invite_contact_clicked(self, *args):
        """
        Sends an INVITE to the remote peer.
        """
        if self.app.has_session():
            self.app.stop_streamers()
        else:
            self.app.send_invite()

    def show_confirm_dialog(self, text, callback=None):
        """
        This could be replaced by a yes/no dialog. That's actually what it is.
        """
        deferred = self.confirm_dialog.show(text)
        if callback is not None:
            deferred.addCallback(callback)

    def show_invited_dialog(self, text):
        """ 
        This could be replaced by a yes/no dialog. That's actually what it is.
        @rtype: L{Deferred}
        """
        return self.invited_dialog.show(text)

    def show_calling_dialog(self):
        """
        Creates a new widget and show it.
        """
        self.calling_dialog = None
        widgets_tree = glade.get_widgets_tree()
        self.calling_dialog = widgets_tree.get_widget("calling_dialog")
        # FIXME: can't set a parent on a toplevel window
        #self.calling_dialog.set_parent(self.main_window)
        self.calling_dialog.connect('delete-event', self.on_invite_contact_cancelled)
        self.calling_dialog.show()
    
    def on_invite_contact_cancelled(self, *args):
        """
        Sends a CANCEL to the remote peer when invite contact window is closed.
        """
        # unschedule this timeout as we don't care if our peer answered or not
        self.app.send_cancel_and_disconnect(reason=communication.CANCEL_REASON_CANCELLED)
        print("Inviting window is closed. ")
        self.hide_calling_dialog()
        return True # don't let the delete-event propagate

    def hide_calling_dialog(self):
        """
        Hides the "calling_dialog" dialog.
        """
        self._unschedule_inviting_timeout_delayed()
        if self.calling_dialog is not None:
            self.calling_dialog.hide()

    def _unschedule_inviting_timeout_delayed(self):
        """
        Unschedules our offer timeout delayed call. 
        """
        if self._inviting_timeout_delayed is not None and self._inviting_timeout_delayed.active():
            self._inviting_timeout_delayed.cancel()
            self._inviting_timeout_delayed = None
    
    def _schedule_inviting_timeout_delayed(self):
        """ 
        Schedules our offer invite timeout function 
        """
        def _cl_offerer_invite_timed_out():
            # in case of invite timeout, act as if we'd cancelled the invite ourselves
            print("Inviting window time out. ")
            self.app.send_cancel_and_disconnect(reason=communication.CANCEL_REASON_TIMEOUT)
            self.on_invite_contact_cancelled()
            self.hide_calling_dialog()
            text = _("The invitation expired. \n\nThe remote peer did not answer quick enough.")
            dialogs.ErrorDialog.create(text, parent=self.main_window)
            # here we return false so that this callback is unregistered
            return False

        if self._inviting_timeout_delayed is None or not self._inviting_timeout_delayed.active():
            self._inviting_timeout_delayed = reactor.callLater(INVITE_TIMEOUT, _cl_offerer_invite_timed_out)
        else:
            print("Warning: Already scheduled a timeout as we're already inviting a contact")

    def update_jackd_status(self):
        is_zombie = self.app.devices["jackd_is_zombie"]
        is_running = self.app.devices["jackd_is_running"]
        fill_stats = False
        if is_zombie:
                self.audio_jack_state_widget.set_markup(_("<b>Zombie</b>"))
                self.audio_jack_icon_widget.set_from_stock(gtk.STOCK_DIALOG_WARNING, 4)
        else:
            if is_running:
                self.audio_jack_state_widget.set_markup(_("<b>Running</b>"))
                self.audio_jack_icon_widget.set_from_stock(gtk.STOCK_YES, 4)
                fill_stats = True
            else:
                self.audio_jack_state_widget.set_markup(_("<b>Not running</b>"))
                self.audio_jack_icon_widget.set_from_stock(gtk.STOCK_NO, 4)
        if fill_stats:
            j = self.app.devices["jack_servers"][0] 
            try:
                latency = (j["period"] * j["nperiods"] / float(j["rate"])) * 1000 # ms
            except KeyError, e:
                print 'Key %s is missing for the jack server process' % (e)
            else:
                self.jack_latency_widget.set_text("%4.2f ms" % (latency))
                self.jack_sampling_rate_widget.set_text("%d Hz" % (j["rate"]))
        else:
            self.jack_latency_widget.set_text("")
            self.jack_sampling_rate_widget.set_text("")
            

class About(object):
    """
    About dialog
    """
    def __init__(self):
        # TODO: set parent window ?
        self.icon_file = os.path.join(configure.PIXMAPS_DIR, 'scenic.png')
        self.about_dialog = gtk.AboutDialog()

    def show_about_dialog(self):
        self.about_dialog.set_name(configure.APPNAME)
        self.about_dialog.set_role('about')
        self.about_dialog.set_version(configure.VERSION)
        commentlabel = ONE_LINE_DESCRIPTION 
        self.about_dialog.set_comments(commentlabel)
        self.about_dialog.set_copyright(COPYRIGHT_SHORT) 
        self.about_dialog.set_license(LICENSE_TEXT)
        self.about_dialog.set_authors(AUTHORS_LIST)
        #self.about_dialog.set_artists(['Public domain'])
        gtk.about_dialog_set_url_hook(self.show_website)
        self.about_dialog.set_website(PROJECT_WEBSITE)
        if not os.path.exists(self.icon_file):
            print("Could not find icon file %s." % (self.icon_file))
        else:
            large_icon = gtk.gdk.pixbuf_new_from_file(self.icon_file)
            self.about_dialog.set_logo(large_icon)
        # Connect to callbacks
        self.about_dialog.connect('response', self.destroy_about)
        self.about_dialog.connect('delete_event', self.destroy_about)
        self.about_dialog.show_all()

    @staticmethod
    def create():
        """
        @rtype: None
        """
        dialog = About()
        return dialog.show_about_dialog()
     
    def show_website(self, widget, data):
        webbrowser.open(data)

    def destroy_about(self, *args):
        self.about_dialog.destroy()

