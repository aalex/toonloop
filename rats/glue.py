#!/usr/bin/env python
"""
Checking for OpenGL extensions using glew
"""
import pyglew
pyglew.glewInit()

all = [
    pyglew.GLEW_ARB_vertex_shader, 
    pyglew.GLEW_ARB_shader_objects, 
    pyglew.GLEW_ARB_fragment_shader, 
    pyglew.GLEW_ARB_texture_rectangle, 
    pyglew.GLEW_ARB_shading_language_100, 
    pyglew.GLEW_ARB_multitexture, 
    ]

if __name__ == "__main__":
    for func in all:
        print("%s: %s" % (func.__name__, func()))
