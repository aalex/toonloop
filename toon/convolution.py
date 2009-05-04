#!/usr/bin/env python
"""
Convolution shader using Python

"""
from rats.serialize import Serializable
from rats.glsl import ShaderProgram
from rats.glsl import ShaderError

class ShaderEffect(Serializable):
    """
    Generic GLSL Shader Effect
    """
    name = "default_" # prefix for variables names
    def __init__(self, **variables):
        self.variables = {} # uniform config values
        self.variables.update(config)
        self.vert = "" # vertex shader text
        self.frag = "" # fragment shader text
        self.texture0_id = 0 # texture unit number (most likely 0)
        self.program = None # glsl compiled program
        self.enabled = True

    def set_uniforms(self):
        """Define all uniforms variables here.
        """
        pass

    def program_init(self):
        """At application startup.
        """
        try:
            self.program = ShaderProgram()
            self.program.add_shader_text(GL_VERTEX_SHADER_ARB, self.vert)
            self.program.add_shader_text(GL_FRAGMENT_SHADER_ARB, self.frag)
            self.program.linkShaders()
        except Exception, e: 
            print e.message

    def program_use_begin(self):
        """Before rendering a frame.
        """
        if self.enabled:
            try:
                self.program.enable()
            except Exception, e: 
                print e.message

    def program_use_end(self):
        """After rendering a frame.
        """
        try:
            self.program.disable()
        except Exception, e: 
            print e.message

