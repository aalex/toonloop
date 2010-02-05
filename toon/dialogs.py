#!/usr/bin/env python
"""
GTK dialogs integrated with Twisted.
"""
import pprint
if __name__ == "__main__":
    from twisted.internet import gtk2reactor
    gtk2reactor.install()
import gtk
from twisted.internet import reactor
from twisted.internet import defer


class Save(object):
    def __init__(self, deferred, title="Save...", folder=None, default_file_name=None):
        self.deferredResult = deferred
        parent = None
        filechooser = gtk.FileChooserDialog(
            title,
            parent,
            gtk.FILE_CHOOSER_ACTION_SAVE,
            (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
            gtk.STOCK_SAVE, gtk.RESPONSE_OK))
        if default_file_name is not None:
            filechooser.set_current_name(default_file_name)
        filechooser.set_default_response(gtk.RESPONSE_OK)
        if folder is not None:
            filechooser.set_current_folder(folder)
        # connecting signals
        filechooser.connect("close", self.on_close)
        filechooser.connect("response", self.on_response)
        filechooser.show()

    def on_close(self, dialog, *params):
        print("on_close %s %s" % (dialog, params))

    def on_response(self, dialog, response_id, *params):
        #print("on_response %s %s %s" % (dialog, response_id, params))
        if response_id == gtk.RESPONSE_DELETE_EVENT:
            print("Deleted")
        elif response_id == gtk.RESPONSE_CANCEL:
            print("Cancelled")
            self.terminate(dialog, None)
        elif response_id == gtk.RESPONSE_OK:
            print("Accepted")
            file_name = dialog.get_filename()
            self.terminate(dialog, file_name)

    def terminate(self, dialog, file_name):
        dialog.destroy()
        self.deferredResult.callback(file_name)

class ErrorDialog(object):
    """
    Error dialog. Fires the deferred given to it once done.
    Use the create static method as a factory.
    """
    def __init__(self, deferred, message, parent=None):
        self.deferredResult = deferred
        error_dialog = gtk.MessageDialog(
            parent=parent, 
            flags=0, 
            type=gtk.MESSAGE_ERROR, 
            buttons=gtk.BUTTONS_CLOSE, 
            message_format=message)
        error_dialog.set_modal(True)
        error_dialog.connect("close", self.on_close)
        error_dialog.connect("response", self.on_response)
        error_dialog.show()

    @staticmethod
    def create(message, parent=None):
        """
        Returns a Deferred which will be called with a True result.
        @param message: str
        @rettype: L{Deferred}
        """
        d = defer.Deferred()
        dialog = ErrorDialog(d, message, parent)
        return d

    def on_close(self, dialog, *params):
        print("on_close %s %s" % (dialog, params))

    def on_response(self, dialog, response_id, *params):
        #print("on_response %s %s %s" % (dialog, response_id, params))
        if response_id == gtk.RESPONSE_DELETE_EVENT:
            print("Deleted")
        elif response_id == gtk.RESPONSE_CANCEL:
            print("Cancelled")
        elif response_id == gtk.RESPONSE_OK:
            print("Accepted")
        self.terminate(dialog)

    def terminate(self, dialog):
        dialog.destroy()
        self.deferredResult.callback(True)

class YesNoDialog(object):
    """
    Yes/no confirmation dialog.
    Use the create static method as a factory.
    """
    def __init__(self, deferred, message, parent=None):
        self.deferredResult = deferred
        dialog = gtk.MessageDialog(
            parent=parent, 
            flags=0, 
            type=gtk.MESSAGE_QUESTION, 
            buttons=gtk.BUTTONS_YES_NO, 
            message_format=message)
# gtk.BUTTONS_YES_NO: (gtk.STOCK_NO, gtk.RESPONSE_NO,
# gtk.STOCK_YES, gtk.RESPONSE_YES),
# image = gtk.STOCK_DIALOG_QUESTION
        dialog.set_modal(True)
        dialog.connect("close", self.on_close)
        dialog.connect("response", self.on_response)
        dialog.show()

    @staticmethod
    def create(message, parent=None):
        parent = None
        """
        Returns a Deferred which will be called with a boolean result.
        @param message: str
        @rettype: L{Deferred}
        """
        d = defer.Deferred()
        dialog = YesNoDialog(d, message, parent)
        return d

    def on_close(self, dialog, *params):
        print("on_close %s %s" % (dialog, params))

    def on_response(self, dialog, response_id, *params):
        print("on_response %s %s %s" % (dialog, response_id, params))
        if response_id == gtk.RESPONSE_DELETE_EVENT:
            print("Deleted")
            self.terminate(dialog, False)
        elif response_id == gtk.RESPONSE_NO:
            print("Cancelled")
            self.terminate(dialog, False)
        elif response_id == gtk.RESPONSE_YES:
            print("Accepted")
            self.terminate(dialog, True)

    def terminate(self, dialog, answer):
        dialog.destroy()
        self.deferredResult.callback(answer)

def _non_blocking_save_dialog():
    def _on_result(result):
        print(result)
    deferred = defer.Deferred()
    deferred.addCallback(_on_result)
    Save(deferred)
    return deferred

# deprecated
def _error_dialog(mess):
    def _later():
        def _cb(result):
            reactor.stop()
        deferred = defer.Deferred()
        deferred.addCallback(_cb)
        ErrorDialog(deferred, mess)
    reactor.callLater(0.1, _later)
    reactor.run()
   
def _save():
    def _later():
        def _cb(result):
            reactor.stop()
        deferred = _non_blocking_save_dialog()
        deferred.addCallback(_cb)
    reactor.callLater(0.1, _later)
    reactor.run()

if __name__ == "__main__":
    #_save()
    #mess = """An error happened. You are either in big troubles, or in very big troubles."""
    #_error_dialog(mess)
    
    def _cb(result):
        print "result", result
        reactor.stop()
    def _later():
        d = YesNoDialog.create("Really quit ?")
        d.addCallback(_cb)
    reactor.callLater(0, _later)
    reactor.run()
