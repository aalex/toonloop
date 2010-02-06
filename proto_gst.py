#!/usr/bin/env python
"""
Testing OpenGL in a GTK Window.

This is quite long to startup, though.
"""
# TODO: screenshot !
# screenshot = glReadPixels( 0,0, 800, 600, GL_RGBA, GL_UNSIGNED_BYTE)
# im = Image.frombuffer("RGBA", (800,600), screenshot, "raw", "RGBA", 0, 0)
# im.save("test.jpg")


import os
import sys
if __name__ == "__main__":
    from twisted.internet import gtk2reactor
    gtk2reactor.install()
from twisted.internet import reactor
import pygtk
pygtk.require('2.0')
import gtk
import gtk.gtkgl
from OpenGL.GL import *
from OpenGL.GLU import *
import gst
import struct
import Image # PIL
import numpy
import time

WIDTH = 640
HEIGHT = 480
DATA_PATH = "/var/tmp/toonloop"

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
        #self.texture_id = None
        self.pil_image_texture = None
        

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

        self.pil_image_texture = Texture()
        #self.pil_image_texture.load_image_to_texture("./example.jpg")

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
        #if self.texture_id is None:
        #    self._create_texture()
        gldrawable.gl_end()
        return False

    def _on_expose_event(self, *args):
        self._draw()
    
    def _draw(self):
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
        reactor.callLater(0.05, self._draw) # FIXME
        return False
    
#    def _create_texture(self):
#        black_pixel = struct.pack(">i", 255)
#        pixels = black_pixel * 320 * 240 # 32 bits black
#        w = 320
#        h = 240
#        
#        # Create Texture
#        tex_id = glGenTextures(1)
#        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_id)
#
#        glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
#        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, w, h, 0,
#                     GL_RGBA, GL_UNSIGNED_BYTE, pixels)
#        glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
#        glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
#        self.texture_id = tex_id

    def draw(self):
        """
        Draws each frame.
        """
        sys.stdout.write('-')
        sys.stdout.flush()
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

#        if self.texture_id is not None:
#            glColor4f(1.0, 1.0, 1.0, 1.0)
#            glEnable(GL_TEXTURE_RECTANGLE_ARB)
#            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.texture_id)
#            glPushMatrix()
#            glScale(0.4, 0.3, 1.0)
#            draw_textured_square(320, 240)
#            glPopMatrix()
#        else:
#            print "No texture to draw"
        if self.pil_image_texture is not None:
            glColor4f(1.0, 1.0, 1.0, 1.0)
            glEnable(GL_TEXTURE_RECTANGLE_ARB)
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.pil_image_texture.texture_id)
            glPushMatrix()
            glTranslate(0.4, 0, 0)
            glScale(0.4, 0.3, 1.0)
            draw_textured_square(320, 240)
            glPopMatrix()

        draw_line(0., 0., 1., 1.)

class Texture(object):
    """
    Loads a texture from an image file.
    """
    def __init__(self):
        self.width = 320
        self.height = 240
        self.image_mode = "RGB"
        
        image_data = numpy.array([127 for i in range(self.width * self.height)])
        
        # Create Texture
        self.texture_id = glGenTextures(1)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.texture_id)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 
            0, GL_RGB, 
            self.width, self.height, 
            0, GL_RGB, 
            GL_UNSIGNED_BYTE, image_data)

    def load_image_to_texture(self, file_name):
        try:
            print 'opening ', file_name
            pil_image = Image.open(file_name)
            self.width = pil_image.size[0]
            self.height = pil_image.size[1]
            self.image_mode = pil_image.mode
            image_data = pil_image.tostring("raw", pil_image.mode, 0, -1)
            #image_data = numpy.array(list(pil_image.getdata()), numpy.int8)
        except Exception, e:
            print e
        else:
            print 'binding texture %sx%s %s' % (self.width, self.height, self.image_mode)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.texture_id)
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
            if self.image_mode == "RGB":
                print 'rgb'
                try:
                    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 
                        0, # base level 
                        0, 0, # x y offset
                        self.width,self.height, 
                        GL_RGB, GL_UNSIGNED_BYTE,
                        image_data)
                except Exception, e:
                    print e
                    #glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,
                    #    0, 3, self.width, self.height, 0, GL_RGB, GL_UNSIGNED_BYTE, self.image_data)
            #elif self.image_mode == "RGBA":
            #    print 'rgba'
            #    try:
            #        glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 
            #            0, 0, 0, 
            #            self.width, self.height, 
            #            GL_RGBA, GL_UNSIGNED_BYTE, self.image_data)
            #    except Exception, e:
            #        print e
            #        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,
            #            0, 4, self.width, self.height, 
            #            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            #            self.image_data)
            #else:
            #    print "image mode not implemented:", self.image_mode
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
            

class App(object):
    """
    Main window of the application.
    """
    def __init__(self):
        """
        Creates the drawing area and other widgets.
        """
        self.is_fullscreen = False
        self.incrementing_image_number = 0
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
        elements = []
        #self.source = gst.element_factory_make('videotestsrc', 'source_1')
        self.source = gst.element_factory_make('v4l2src', 'source_1')
        elements.append(self.source)
        caps = gst.Caps("video/x-raw-yuv, width=320, height=240")
        filter = gst.element_factory_make("capsfilter", "filter")
        filter.set_property("caps", caps)
        elements.append(filter)
        self.color_fix = gst.element_factory_make('ffmpegcolorspace', 'color_fix')
        elements.append(self.color_fix)
        self.queue_element = gst.element_factory_make('queue', 'relax')
        elements.append(self.queue_element)
        self.pixbuffer = gst.element_factory_make('gdkpixbufsink', 'snapshot')
        elements.append(self.pixbuffer)
        self.pipeline.add(*elements)
        gst.element_link_many(*elements)
        #self.bus = self.pipeline.get_bus()
        #self.bus.add_signal_watch()
        #self.bus.connect('message', self.on_bus_message)
        self.pipeline.set_state(gst.STATE_PLAYING)

    def __del__(self):
        self.pipeline.set_state(gst.STATE_NULL)

    #def on_bus_message(self, bus, message):
    #    #print "bus,mess:", bus, message
    #    t = message.type
    #    if t == gst.MESSAGE_ELEMENT and message.structure.get_name() == 'pixbuf':
    #        pixbuf = message.structure['pixbuf']
    #        #pixbuf = self.pixbuffer.get_property('last-pixbuf')
    #        #print "size:", pixbuf.get_width(), pixbuf.get_height()
    #        self._update_texture(pixbuf)

    def on_key_pressed(self, widget, event):
        """
        Escape toggles fullscreen mode.
        Space grabs an image.
        """
        name = gtk.gdk.keyval_name(event.keyval)
        #if self.verbose:
        print("%s pressed" % (name))
        if name == "space":
            print 'space pressed'
            pixbuf = self.pixbuffer.get_property('last-pixbuf')
            print pixbuf
            try:
                print "grabbing size:", pixbuf.get_width(), pixbuf.get_height()
                # TODO:try/except GError
                file_name = os.path.join(DATA_PATH, "snapshot_%d.jpg" % (self.incrementing_image_number))
                pixbuf.save(file_name, "jpeg", {"quality":"100"})
                #time.sleep(0.1)
                if self.drawing_area.pil_image_texture is not None:
                    print "loading the image"
                    self.drawing_area.pil_image_texture.load_image_to_texture(file_name)
                self.incrementing_image_number += 1
            except AttributeError, e:
                print str(e)
        elif name == "Escape":
            self.toggle_fullscreen()
        elif name == "q":
            #self.window.destroy()
            reactor.stop()
        return True
        
    #def _update_texture(self, image):
    #    #if self.drawing_area.texture_id is not None:
    #    #print("updating texture")
    #    sys.stdout.write(".")
    #    sys.stdout.flush()
    #    pixels = image.get_pixels()
    #    w = image.get_width() # 320
    #    h = image.get_height() # 240
    #    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, self.drawing_area.texture_id)
    #    # 24 bits means it's RGBA. TODO: change the caps
    #    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, w, h, 0,
    #        GL_RGBA, GL_UNSIGNED_BYTE, pixels)
    #    glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
    #    glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    #    glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
 
    def on_delete_event(self, widget, event=None):
        """
        Closing the window quits.
        """
        reactor.stop()

    def on_quit_clicked(self, widget, event=None):
        """
        The quit button quits.
        """
        self.window.destroy()
        
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


if __name__ == '__main__':
    if not os.path.exists(DATA_PATH):
        os.makedirs(DATA_PATH)
    print "screen is %sx%s" % (gtk.gdk.screen_width(), gtk.gdk.screen_height())
    app = App()
    reactor.run()
