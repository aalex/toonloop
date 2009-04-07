#!/usr/bin/env python
"""
GLSL shaders with SDL, OpenGL texture and Python
"""
# ----------- GLSL tools ----------
from ctypes import *
import sys
 
try:
    # For OpenGL-ctypes
    from OpenGL import platform
    gl = platform.OpenGL
except ImportError:
    try:
        # For PyOpenGL
        gl = cdll.LoadLibrary('libGL.so')
    except OSError:
        # Load for Mac
        from ctypes.util import find_library
        # finds the absolute path to the framework
        path = find_library('OpenGL')
        gl = cdll.LoadLibrary(path)
 
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
 
glCreateShader = gl.glCreateShader
glShaderSource = gl.glShaderSource
glShaderSource.argtypes = [c_int, c_int, POINTER(c_char_p), POINTER(c_int)]
glCompileShader = gl.glCompileShader
glGetShaderiv = gl.glGetShaderiv
glGetShaderiv.argtypes = [c_int, c_int, POINTER(c_int)]
glGetShaderInfoLog = gl.glGetShaderInfoLog
glGetShaderInfoLog.argtypes = [c_int, c_int, POINTER(c_int), c_char_p]
glDeleteShader = gl.glDeleteShader
glCreateProgram = gl.glCreateProgram
glAttachShader = gl.glAttachShader
glLinkProgram = gl.glLinkProgram
glGetError = gl.glGetError
glUseProgram = gl.glUseProgram
#Get two more functions out of libGL using ctypes:
glGetUniformLocation = gl.glGetUniformLocation
glUniform1i = gl.glUniform1i
 
GL_FRAGMENT_SHADER = 0x8B30
GL_VERTEX_SHADER = 0x8B31
GL_COMPILE_STATUS = 0x8B81
GL_LINK_STATUS = 0x8B82
GL_INFO_LOG_LENGTH = 0x8B84
 
def compile_shader(source, shader_type):
    """
    Compile either a vertex or fragment GLSL code to a shader.
    """
    shader = glCreateShader(shader_type)
    source = c_char_p(source)
    length = c_int(-1)
    glShaderSource(shader, 1, byref(source), byref(length))
    glCompileShader(shader)
    status = c_int()
    glGetShaderiv(shader, GL_COMPILE_STATUS, byref(status))
    if not status.value:
        print_log(shader)
        glDeleteShader(shader)
        raise ValueError, 'Shader compilation failed'
    return shader
 
def compile_program(vertex_source, fragment_source):
    """
    Compiles a GLSL program from its vertex and fragment components.
    """
    vertex_shader = None
    fragment_shader = None
    program = glCreateProgram()
    if vertex_source:
        vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER)
        glAttachShader(program, vertex_shader)
    if fragment_source:
        fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER)
        glAttachShader(program, fragment_shader)
    glLinkProgram(program)
    if vertex_shader:
        glDeleteShader(vertex_shader)
    if fragment_shader:
        glDeleteShader(fragment_shader)
 
    return program
 
def print_log(shader):
    """
    Print GLSL OpenGL error to stderr is there is one.
    """
    length = c_int()
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, byref(length))
    if length.value > 0:
        log = create_string_buffer(length.value)
        glGetShaderInfoLog(shader, length, byref(length), log)
        print >> sys.stderr, log.value
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

void main() {
     gl_FragColor.rgb = mix(pos.xyz, tex2D(texture, gl_TexCoord[0].xy).rgb, 0.5);
}
"""

# ------------------------------ CAMERA STUFF -------------
"""
OpenGL camera with toggle fullscreen. 
Trying to get orthographic projection to work
TODO : add a shader.
# 1. Basic image capturing and displaying using the camera module
"""
import os
import pygame
import pygame.image
import pygame.camera
from pygame.locals import *

textures = [0] # list of texture ID 
program = None

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
    glOrtho(-1.333333, 1.333333, -1.0, 1.0, -1.0, 1.0)# aalex just added this
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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    glUseProgram(program)
    #Set the sampler to use the first texture unit (0):
    texture_param = glGetUniformLocation(program, "texture");
    glUniform1i(texture_param, 0);
    
    glPushMatrix()
    glBegin(GL_QUADS)
    glTexCoord2f(0.0, 0.0)
    glVertex2f(-1.33333, -1.0) # Bottom Left
    glTexCoord2f(1.0, 0.0)
    glVertex2f( 1.33333, -1.0) # Bottom Right
    glTexCoord2f(1.0, 1.0)
    glVertex2f( 1.33333,  1.0) # Top Right
    glTexCoord2f(0.0, 1.0)
    glVertex2f(-1.33333,  1.0) # Top Left
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
