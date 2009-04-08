#!/usr/bin/env python
"""
GLSL shaders utilities
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
# from OpenGL.GLUT import *
 
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
#Get two more functions for GLSL shaders using ctypes:
glGetUniformLocation = gl.glGetUniformLocation
glUniform1i = gl.glUniform1i
# glUniform2i = gl.glUniform2i
# glUniform3i = gl.glUniform3i
# glUniform1f = gl.glUniform1f
# glUniform2f = gl.glUniform2f
# glUniform3f = gl.glUniform3f
 
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

def load_file_contents(file_name):
    """
    Returns the contents of a file
    """
    try:
        f = open(file_name, 'r')
    except IOError, e:
        print e.message
        raise
    else:
        contents = f.read()
        f.close()
        return contents

# Taken from http://swiftcoder.wordpress.com/2008/12/19/simple-glsl-wrapper-for-pyglet/
#  
# Copyright Tristam Macdonald 2008.  
#  
# Distributed under the Boost Software License, Version 1.0  
# (see http://www.boost.org/LICENSE_1_0.txt)  
#  
def uniformf(program, name, *vals):  
    """
    uploads a floating point uniform  
    this program must be currently bound  
    """
    # check there are 1-4 values  
    if len(vals) in range(1, 5):  
        # select the correct function  
        { 1 : glUniform1f,  
          2 : glUniform2f,  
          3 : glUniform3f,  
          4 : glUniform4f  
          # retrieve the uniform location, and set  
        }[len(vals)](glGetUniformLocation(program, name), *vals)

def uniformi(program, name, *vals):    
    """
    Uploads an integer uniform  
    this program must be currently bound  
    """
    # check there are 1-4 values  
    if len(vals) in range(1, 5):  
        # select the correct function  
        { 1 : glUniform1i,  
          2 : glUniform2i,  
          3 : glUniform3i,  
          4 : glUniform4i  
          # retrieve the uniform location, and set  
        }[len(vals)](glGetUniformLocation(program, name), *vals)  

