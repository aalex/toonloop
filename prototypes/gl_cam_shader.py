#!/usr/bin/env python
"""
OpenGL camera with toggle fullscreen. 
Trying to get orthographic projection to work
TODO : add a shader.
# 1. Basic image capturing and displaying using the camera module
"""
import os
import pygame
import pygame.camera
from pygame.locals import *
from OpenGL.GL import *
#from OpenGL.GLU import *

textures = [0, 0]

def resize((width, height)):
    """
    Called when we resize the window.
    (fullscreen on/off)
    """
    if height == 0:
        height = 1
    #glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    # OLD : gluPerspective(45, 1.0 * width / height, 0.1, 100.0) # 45 degrees camera (like a human)
    # NOTE : gluOrtho2D sets up a two-dimensional orthographic viewing region. This is equivalent to calling glOrtho with near=-1 and far=1.
    glOrtho(-1.333333, 1.333333, -1.0, 1.0, -1.0, 1.0)# aalex just added this
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def gl_init():
    """
    Init the window
    """
    glEnable(GL_TEXTURE_2D)
    glShadeModel(GL_SMOOTH)
    glClearColor(0.0, 0.0, 0.0, 0.0) # black background
    #glClearDepth(1.0)
    #glEnable(GL_DEPTH_TEST)
    #glDepthFunc(GL_LEQUAL)
    #glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)

def draw():
    """
    Called on every frame rendering
    """
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    #glLoadIdentity()
    #glTranslatef(0.0, 0.0, -5.0)
    #glOrtho(0, 1, 0, 1, -1, 1) # aalex just added this

    glPushMatrix()
    glBegin(GL_QUADS)
    # Front Face (note that the texture's corners have to match the quad's corners)
    glTexCoord2f(0.0, 0.0); glVertex2f(-1.33333, -1.0) # Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0, 0.0); glVertex2f( 1.33333, -1.0) # Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0, 1.0); glVertex2f( 1.33333,  1.0) # Top Right Of The Texture and Quad
    glTexCoord2f(0.0, 1.0); glVertex2f(-1.33333,  1.0) # Top Left Of The Texture and Quad
    glEnd()
    glPopMatrix()

class VideoCapturePlayer(object):
    size = ( 640, 480 )
    def __init__(self, **argd):
        self.__dict__.update(**argd)
        super(VideoCapturePlayer, self).__init__(**argd)

        # create a display surface. standard pygame stuff
        self.screen = pygame.display.set_mode( self.size, OPENGL|DOUBLEBUF|HWSURFACE)
        pygame.display.set_caption("GLCamera")
        resize(self.size)
        gl_init()

        # gets a list of available cameras.
        self.clist = pygame.camera.list_cameras()
        if not self.clist:
            raise ValueError("Sorry, no cameras detected.")
        
        if os.uname()[0] == 'Darwin':
            self.isMac = True
            # creates the camera of the specified size and in RGB colorspace
            self.camera = pygame.camera.Camera(0, self.size, 'RGBA')
        else:
            self.isMac = False
            # creates the camera of the specified size and in RGB colorspace
            self.camera = pygame.camera.Camera('/dev/video0', self.size, "RGBA")

    
        # starts the camera
        self.camera.start()
        self.clock = pygame.time.Clock()

        # create a surface to capture to.  for performance purposes, you want the
        # bit depth to be the same as that of the display surface.
        self.snapshot = pygame.surface.Surface(self.size, 0, self.screen)

    def get_and_flip(self):
        """
        Grabs a frame from the camera (to a texture) and renders the screen.

        if you don't want to tie the framerate to the camera, you can check and
        see if the camera has an image ready.  note that while this works
        on most cameras, some will never return true.
        """
        if 0 and self.camera.query_image():
            # capture an image
            self.snapshot = self.camera.get_image(self.snapshot)
        
        self.snapshot = self.camera.get_image(self.snapshot)
        textureData = pygame.image.tostring(self.snapshot, "RGBX", 1)
    
        glBindTexture(GL_TEXTURE_2D, textures[0])
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, self.snapshot.get_width(), self.snapshot.get_height(), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, textureData )
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        draw()
        pygame.display.flip()
    
    def main(self):
        going = True
        while going:
            events = pygame.event.get()
            for e in events:
                if e.type == QUIT or (e.type == KEYDOWN and e.key == K_ESCAPE):
                    going = False
                elif e.type == KEYDOWN:
                    if e.key == K_f:
                        pygame.display.toggle_fullscreen()

            self.get_and_flip()
            self.clock.tick(30)
            #print "FPS:", self.clock.get_fps()

def main():
    pygame.init()
    pygame.camera.init()
    VideoCapturePlayer().main()
    pygame.quit()

if __name__ == '__main__':
    main()
