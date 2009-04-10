#!/usr/bin/env python
"""
GLSL shaders with SDL, OpenGL texture and Python
"""
import os
import sys
import pygame
import pygame.image
import pygame.camera
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
from rats.glsl import *
# from OpenGL.arrays.arraydatatype import GLfloatArray
# ---------------------------- glsl ----------------------------
#The vertex program needs to pass the texture coordinates through: 
#(should multiply by the texture matrix here, but we don't care about that)
vert = """
// Vertex program
varying vec3 pos;

void main() {
     pos = gl_Vertex.xyz;
     gl_TexCoord[0] = gl_MultiTexCoord0;
     gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
"""

#Add a uniform sampler2D to the fragment shader, and make it modulate the 
#colour with the texture:
frag = """
// Fragment program
varying vec3 pos;
uniform sampler2D texture;
uniform float mixer;
varying vec3 offset;

void main() {
     gl_FragColor.rgb = mix(pos.xyz, tex2D(texture, gl_TexCoord[0].xy).rgb, mixer) + offset;
     //gl_FragColor.rgb = mix(tex2D(texture, gl_TexCoord[0].xy).rgb, offset, mixer);
}
"""

# ------------------------------ CAMERA STUFF -------------
"""
OpenGL camera with toggle fullscreen. 
Trying to get orthographic projection to work
TODO : add a shader.
# 1. Basic image capturing and displaying using the camera module
"""

textures = [0] # list of texture ID 
program = None
mixer_ratio = 0.0 # from 0.0 to 1.0
color_offset = 0.0
def resize((width, height)):
    """
    Called when we resize the window.
    (fullscreen on/off)
    """
    if height == 0:
        height = 1
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    # NOTE : gluOrtho2D sets up a two-dimensional orthographic viewing region. This is equivalent to calling glOrtho with near=-1 and far=1.
    glOrtho(-4.0, 4.0, -3.0, 3.0, -1.0, 1.0)# aalex just added this
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def gl_init():
    """
    Init the window
    """
    global program 
    global textures

    glEnable(GL_TEXTURE_2D)
    glShadeModel(GL_SMOOTH)
    textures[0] = glGenTextures(1)
    glClearColor(0.0, 0.0, 0.0, 0.0) # black background
    program = compile_program(vert, frag)

def draw():
    """
    Called on every frame rendering
    """
    global program 
    global textures
    global mixer_ratio
    global color_offset

    mixer_ratio = (mixer_ratio + 0.01) % 1.0
    color_offset = (color_offset + 0.1) % 1.0
    #print "mixer:", mixer_ratio

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    glUseProgram(program)
    #Set the sampler to use the first texture unit (0):
    texture_param = glGetUniformLocation(program, "texture");
    #print "texture uniform location :", texture_param
    glUniform1i(texture_param, 0);
    #print "mixer uniform location: ", glGetUniformLocation(program, "mixer")
    glUniform1f(glGetUniformLocation(program, "mixer"),  GLfloat(mixer_ratio))
    #glUniform3fv(glGetUniformLocation(program, "offset"), 3, GLfloat(color_offset), GLfloat(0.0), GLfloat(0.0))
    glUniform3f(glGetUniformLocation(program, "offset"), GLfloat(mixer_ratio), GLfloat(0.0), GLfloat(0.0))
    #glUniform3fv( GLint(program), GLsizei(3), GLfloatArray([1.0, 0, 0]) )
    #sys.stdout.write("%0.3f " % color_offset)
    #sys.stdout.flush()
    glPushMatrix()
    glBegin(GL_QUADS)
    glTexCoord2f(0.0, 0.0)
    glVertex2f( -4.0, -3.0) # Bottom Left
    glTexCoord2f(1.0, 0.0)
    glVertex2f(  4.0, -3.0) # Bottom Right
    glTexCoord2f(1.0, 1.0)
    glVertex2f(  4.0,  3.0) # Top Right
    glTexCoord2f(0.0, 1.0)
    glVertex2f( -4.0,  3.0) # Top Left
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
        global textures

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
