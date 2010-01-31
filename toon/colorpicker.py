from twisted.internet import gtk2reactor
gtk2reactor.install()
from twisted.internet import reactor
from twisted.internet import defer
import gtk

class ColorDialog(object):
    """
    Color Selection dialog.
    Use the create static method as a factory.
    """
    def __init__(self, deferred, text):
        self.deferredResult = deferred
        parent = None
        color_dialog = gtk.ColorSelectionDialog(text)
        color_dialog.get_color_selection().connect("color-changed", self.on_changing)
        self.color_dialog = color_dialog
        #color_dialog.set_modal(True)
        color_dialog.ok_button.connect("clicked", self.on_chosen)
        color_dialog.cancel_button.connect("clicked", self.on_none)
        color_dialog.help_button.connect("clicked", self.on_help)
        color_dialog.show()

    @staticmethod
    def create(message):
        """
        Returns a Deferred which will be called with a boolean result.
        @param message: str
        @rettype: L{Deferred}
        """
        d = defer.Deferred()
        dialog = ColorDialog(d, message)
        return d

    def _color_to_gl(self, color):
        return (color.red / 65535.0, color.green / 65535.0, color.blue / 65535.0)
    def on_changing(self, colorselection):
        color = colorselection.get_current_color()
        print self._color_to_gl(color)
        
    def on_chosen(self, dialog, *params):
        print("on_chosen %s %s" % (dialog, params))
        color = self.color_dialog.get_color_selection().get_current_color()
        print color
        result = self._color_to_gl(color)
        self.terminate(result)

    def on_none(self, *args):
        self.terminate(None)

    def terminate(self, answer):
        self.color_dialog.destroy()
        self.deferredResult.callback(answer)

    def on_help(self, *args):
        print "help"
        return True # ?

def _later():
    def _cb(result):
        print result
        reactor.stop()
    d = ColorDialog.create("Pick a color...")
    d.addCallback(_cb)

reactor.callLater(0, _later)
reactor.run()

