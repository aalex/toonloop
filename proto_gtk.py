#!/usr/bin/env python
"""
Testing OpenGL in a GTK Window.

This is quite long to startup, though.
"""
import sys
import pygtk
pygtk.require('2.0')
import gtk
import gtk.gtkgl
from OpenGL.GL import *
from OpenGL.GLU import *
import gst
import struct

WIDTH = 640
HEIGHT = 480

def draw_square():
    """
    Draws a square of 2 x 2 size centered at 0, 0
    
    Make sure to call glDisable(GL_TEXTURE_RECTANGLE_ARB) first.
    """
    glBegin(GL_QUADS)
    glVertex2f(-1.0, -1.0) # Bottom Left of Quad
    glVertex2f(1.0, -1.0) # Bottom Right of Quad
    glVertex2f(1.0, 1.0) # Top Right Of Quad
    glVertex2f(-1.0, 1.0) # Top Left Of Quad
    glEnd()

def draw_textured_square(w=None, h=None):
    """
    Draws a texture square of 2 x 2 size centered at 0, 0
    
    Make sure to call glEnable(GL_TEXTURE_RECTANGLE_ARB) first.

    :param w: width of the image in pixels
    :param h: height of the image in pixels
    """
    if w is None or h is None:
        glBegin(GL_QUADS)
        glTexCoord2f(0.0, 0.0)
        glVertex2f(-1.0, -1.0) # Bottom Left Of The Texture and Quad
        glTexCoord2f(1.0, 0.0)
        glVertex2f(1.0, -1.0) # Bottom Right Of The Texture and Quad
        glTexCoord2f(1.0, 1.0)
        glVertex2f(1.0, 1.0) # Top Right Of The Texture and Quad
        glTexCoord2f(0.0, 1.0)
        glVertex2f(-1.0, 1.0) # Top Left Of The Texture and Quad
        glEnd()
    else:
        glBegin(GL_QUADS)
        glTexCoord2f(0.0, 0.0)
        glVertex2f(-1.0, -1.0) # Bottom Left
        glTexCoord2f(w, 0.0)
        glVertex2f(1.0, -1.0) # Bottom Right
        glTexCoord2f(w, h)
        glVertex2f(1.0, 1.0) # Top Right
        glTexCoord2f(0.0, h)
        glVertex2f(-1.0, 1.0) # Top Left
        glEnd()

def draw_line(from_x, from_y, to_x, to_y):
    """
    Draws a line between given points.
    """
    glBegin(GL_LINES)
    glVertex2f(from_x, from_y) 
    glVertex2f(to_x, to_y) 
    glEnd()

class GlDrawingArea(gtk.DrawingArea, gtk.gtkgl.Widget):
    """
    OpenGL drawing area for simple demo.
    
    OpenGL-capable gtk.DrawingArea by subclassing
    gtk.gtkgl.Widget mixin.
    """
    def __init__(self, glconfig, app):
        gtk.DrawingArea.__init__(self)
        self._app = app # let's pass it 
        # Set OpenGL-capability to the drawing area
        self.set_gl_capability(glconfig)
        # Connect the relevant signals.
        self.connect_after('realize', self._on_realize)
        self.connect('configure_event', self._on_configure_event)
        self.connect('expose_event', self._on_expose_event)
        self.texture_id = None

    def _on_realize(self, *args):
        """
        Called at the creation of the drawing area.

        Sets up the OpenGL rendering context.
        """
        # Obtain a reference to the OpenGL drawable
        # and rendering context.
        gldrawable = self.get_gl_drawable()
        glcontext = self.get_gl_context()
        # OpenGL begin.
        if not gldrawable.gl_begin(glcontext):
            return

        self._set_view(WIDTH / float(HEIGHT))

        glEnable(GL_TEXTURE_RECTANGLE_ARB) # 2D)
        glEnable(GL_BLEND)
        glShadeModel(GL_SMOOTH)
        glClearColor(0.0, 0.0, 0.0, 1.0) # black background
        glColor4f(1.0, 1.0, 1.0, 1.0) # default color is white
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

        # OpenGL end
        gldrawable.gl_end()

    def _set_view(self, ratio):
        """
        Sets up the orthographic projection.

        Height is always 1.0 in GL modelview coordinates.
        
        Coordinates should give a rendering area height of 1
        and a width of 1.33, when in 4:3 ratio.
        """
        w = ratio
        h = 1.

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(-w, w, -h, h, -1.0, 1.0)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def _on_configure_event(self, *args):
        """
        Called when the drawing area is resized.

        Sets up the OpenGL view port dimensions.
        """
        # Obtain a reference to the OpenGL drawable
        # and rendering context.
        gldrawable = self.get_gl_drawable()
        glcontext = self.get_gl_context()
        if gldrawable is None:
            return False
        # OpenGL begin
        if not gldrawable.gl_begin(glcontext):
            return False
        #print "viewport:", 0, 0, self.allocation.width, self.allocation.height
        #print "window size:", self._app.window.get_size()
        #if self._app.is_fullscreen:
        #    glViewport(0, 0, *self._app.actual_size)
        #else:
        glViewport(0, 0, self.allocation.width, self.allocation.height)
        ratio = self.allocation.width / float(self.allocation.height)
        self._set_view(ratio)
        # OpenGL end
        if self.texture_id is None:
            self._create_texture()
        gldrawable.gl_end()
        return False

    def _on_expose_event(self, *args):
        """
        Called on every frame rendering of the drawing area.

        Calls self.draw() and swaps the buffers.
        """
        # Obtain a reference to the OpenGL drawable
        # and rendering context.
        gldrawable = self.get_gl_drawable()
        glcontext = self.get_gl_context()
        if gldrawable is None:
            return False
        # OpenGL begin
        if not gldrawable.gl_begin(glcontext):
            return False
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()
        
        self.draw()

        # DONE
        if gldrawable.is_double_buffered():
            gldrawable.swap_buffers()
        else:
            glFlush()
        # OpenGL end
        gldrawable.gl_end()
        return False
    
    def _create_texture(self):
        black_pixel = struct.pack(">i", 255)
        pixels = black_pixel * 320 * 240 # 32 bits black
        w = 320
        h = 240
        
        # Create Texture
        tex_id = glGenTextures(1)
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_id)

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels)
        glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        self.texture_id = tex_id

    def draw(self):
        """
        Draws each frame.
        """
        # DRAW STUFF HERE
        glDisable(GL_TEXTURE_RECTANGLE_ARB)
        glColor4f(1.0, 0.8, 0.2, 1.0)
        glPushMatrix()
        glScale(0.5, 0.5, 1.0)
        draw_square()
        glPopMatrix()

        glColor4f(1.0, 1.0, 0.0, 0.8)
        num = 64
        for i in range(num):
            x = (i / float(num)) * 4 - 2
            draw_line(float(x), -2.0, float(x), 2.0)
            draw_line(-2.0, float(x), 2.0, float(x))

        if self.texture_id is not None:
            glColor4f(1.0, 1.0, 1.0, 1.0)
            glEnable(GL_TEXTURE_RECTANGLE_ARB)
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.texture_id)
            glPushMatrix()
            glScale(0.4, 0.3, 1.0)
            draw_textured_square(320, 240)
            glPopMatrix()
        else:
            print "No texture to draw"


class App(object):
    """
    Main window of the application.
    """
    def __init__(self):
        """
        Creates the drawing area and other widgets.
        """
        self.is_fullscreen = False
        self.verbose = True
        self.window = gtk.Window()
        self.window.set_title('Testing OpenGL')
        self.window.set_reallocate_redraws(True)
        self.window.connect('delete_event', self.on_delete_event)
        self.window.connect("key-press-event", self.on_key_pressed)
        self.window.connect("window-state-event", self.on_window_state_event)
        self.window.connect("configure_event", self.on_configure_event)
        self.actual_size = (WIDTH, HEIGHT) # should actually be bigger
        # VBox to hold everything.
        vbox = gtk.VBox()
        self.window.add(vbox)
        # Query the OpenGL extension version.
        print "OpenGL extension version - %d.%d\n" % gtk.gdkgl.query_version()
        # Configure OpenGL framebuffer.
        # Try to get a double-buffered framebuffer configuration,
        # if not successful then try to get a single-buffered one.
        display_mode = (
            gtk.gdkgl.MODE_RGB |
            gtk.gdkgl.MODE_DEPTH |
            gtk.gdkgl.MODE_DOUBLE
            )
        try:
            glconfig = gtk.gdkgl.Config(mode=display_mode)
        except gtk.gdkgl.NoMatches:
            display_mode &= ~gtk.gdkgl.MODE_DOUBLE
            glconfig = gtk.gdkgl.Config(mode=display_mode)
        if self.verbose:
            print "is RGBA:", glconfig.is_rgba()
            print "is double-buffered:", glconfig.is_double_buffered()
            print "is stereo:", glconfig.is_stereo()
            print "has alpha:", glconfig.has_alpha()
            print "has depth buffer:", glconfig.has_depth_buffer()
            print "has stencil buffer:", glconfig.has_stencil_buffer()
            print "has accumulation buffer:", glconfig.has_accum_buffer()
        # Drawing Area
        self.drawing_area = GlDrawingArea(glconfig, self)
        self.drawing_area.set_size_request(WIDTH, HEIGHT)
        vbox.pack_start(self.drawing_area)

        # A quit button.
        button = gtk.Button('Quit')
        button.connect('clicked', self.on_quit_clicked)
        vbox.pack_start(button, expand=False)
        
        self.drawing_area.show()
        button.show()
        vbox.show()
        self.window.show() # not show_all() !

        # setup GST

        self.pipeline = gst.Pipeline('test_pipeline')
        self.source = gst.element_factory_make('videotestsrc', 'source_1')
        self.pixbuffer = gst.element_factory_make('gdkpixbufsink', 'snapshot')
        self.pipeline.add(self.source, self.pixbuffer)
        gst.element_link_many(self.source, self.pixbuffer)
        self.bus = self.pipeline.get_bus()
        self.bus.add_signal_watch()
        self.bus.connect('message', self.on_bus_message)
        self.pipeline.set_state(gst.STATE_PLAYING)

    def on_bus_message(self, bus, message):
        #print "bus,mess:", bus, message
        t = message.type
        if t == gst.MESSAGE_ELEMENT and message.structure.get_name() == 'pixbuf':
            pixbuf = message.structure['pixbuf']
            #pixbuf = self.pixbuffer.get_property('last-pixbuf')
            #print "size:", pixbuf.get_width(), pixbuf.get_height()
            self._update_texture(pixbuf)

    def _update_texture(self, image):
        #if self.drawing_area.texture_id is not None:
        #print("updating texture")
        sys.stdout.write(".")
        sys.stdout.flush()
        pixels = image.get_pixels()
        w = image.get_width() # 320
        h = image.get_height() # 240
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.drawing_area.texture_id)
        # 24 bits means it's RGBA. TODO: change the caps
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, w, h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, pixels)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
        glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
 
    def on_delete_event(self, widget, event=None):
        """
        Closing the window quits.
        """
        gtk.main_quit()

    def on_quit_clicked(self, widget, event=None):
        """
        The quit button quits.
        """
        gtk.main_quit()
        
    def on_key_pressed(self, widget, event):
        """
        Escape toggles fullscreen mode.
        """
        name = gtk.gdk.keyval_name(event.keyval)
        if name == "Escape":
            self.toggle_fullscreen()
        return True

    def toggle_fullscreen(self):
        """
        Toggles the fullscreen mode on/off.
        """
        if self.is_fullscreen:
            self.window.unfullscreen()
            self._showhideWidgets(self.drawing_area, False)
        else:
            self.window.fullscreen()
            self._showhideWidgets(self.drawing_area, True)

    def on_window_state_event(self, widget, event):
        """
        Called when toggled fullscreen.
        """
        #print 'window state event', event.type, event.changed_mask, 
        #print event.new_window_state
        self.is_fullscreen = event.new_window_state & gtk.gdk.WINDOW_STATE_FULLSCREEN != 0
        if self.verbose:
            print('fullscreen %s' % (self.is_fullscreen))
        return True

    def on_configure_event(self, widget, event=None):
        """
        This is where we should measure the window size.
        """
        #print "new size:", self.window.get_size()
        self.actual_size = self.window.get_size()

    def _showhideWidgets(self, except_widget, hide=True):
        """
        Show or hide all widgets in the window except the given
        widget. Used for going fullscreen: in fullscreen, you only
        want the clutter embed widget and the menu bar etc.
        Recursive.
        """
        parent = except_widget.get_parent()
        for c in parent.get_children():
            if c != except_widget:
                #print "toggle %s visibility %s" % (c, hide)
                if hide:
                    c.hide()
                else:
                    c.show()
        if parent == self.window:
            return
        self._showhideWidgets(parent, hide)

    def __del__(self):
        self.pipeline.set_state(gst.STATE_NULL)

if __name__ == '__main__':
    print "screen is %sx%s" % (gtk.gdk.screen_width(), gtk.gdk.screen_height())
    app = App()
    gtk.main()
