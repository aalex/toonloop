#!/usr/bin/env python
"""
Class to simplify the use of GLSL programs.

Copyright (C) 2007  "Peter Roesch" <Peter.Roesch@fh-augsburg.de>

This code is licensed under the PyOpenGL License.
Details are given in the file license.txt included in this distribution.
"""
# Original notice : 
# 
# Class to simplify the incorporation of GLSL programs.
# 
# Copyright (C) 2007  "Peter Roesch" <Peter.Roesch@fh-augsburg.de>
#
# This code is licensed under the PyOpenGL License.
# Details are given in the file license.txt included in this distribution.

import sys

class ShaderError(Exception):
    """
    Any error that might have occured that will make a shader program unusable.
    """
    pass

try:
    from OpenGL.GLUT import *
    from OpenGL.GL import *
    from OpenGL.GLU import *
    from OpenGL.GL.ARB.shader_objects import *
    from OpenGL.GL.ARB.fragment_shader import *
    from OpenGL.GL.ARB.vertex_shader import *
except:
    raise
    # raise ShaderError('Error importing GL / shaders')

class ShaderProgram(object):
    """
    Manages GLSL programs.
    """
    def __init__(self):
        self._requiredExtensions = [
                "GL_ARB_fragment_shader",
                "GL_ARB_vertex_shader",
                "GL_ARB_shader_objects",
                "GL_ARB_shading_language_100",
                "GL_ARB_vertex_shader",
                "GL_ARB_fragment_shader"]
        self.checkExtensions(self._requiredExtensions)
        # TODO: 
        if not bool(glCreateProgramObjectARB):
            raise ShaderError("Impossible to create a shader program object.")
        self._shaderProgramID = glCreateProgramObjectARB()
        self._checkOpenGLError()
        self._programReady = False
        self._isEnabled = False
        self._shaderObjectList = []

    def checkExtensions(self, extensions):
        """
        Checks if all extensions in a list are present.
        """
        for ext in extensions:
            if not ext:
                raise ShaderError("Driver does not support extension %s." % (ext))

    def _checkOpenGLError(self):
        """
        Checks if there are OpenGL error messages.
        """
        err = glGetError()
        if err != GL_NO_ERROR:
            err_text = gluErrorString(err)
            print(err)
            print(err_text)
            raise ShaderError('GLERROR: %s' % (err_text))

    def reset(self):
        """
        Disables and removes all shader programs.
        """
        for shaderID in self._shaderObjectList:
            glDetachObjectARB(self._shaderProgramID, shaderID)
            glDeleteObjectARB(shaderID)
            self._shaderObjectList.remove(shaderID)
            self._checkOpenGLError()
        glDeleteObjectARB(self._shaderProgramID)
        self._checkOpenGLError()
        self._shaderProgramID = glCreateProgramObjectARB()
        self._checkOpenGLError()
        self._programReady = False

    def addShader(self, shaderType, fileName):
        """
        Reads a shader program from a file.

        The program is loaded and compiled.
        """
        sourceString = open(fileName, 'r').read()
        self.add_shader_text(shaderType, sourceString)
    
    def add_shader_text(self, shaderType, sourceString):
        """
        Loads a program from string and compiles it.
        """
        shaderHandle = glCreateShaderObjectARB(shaderType)
        self._checkOpenGLError()
        glShaderSourceARB(shaderHandle, [sourceString])
        self._checkOpenGLError()
        glCompileShaderARB(shaderHandle)
        success = glGetObjectParameterivARB(shaderHandle, 
            GL_OBJECT_COMPILE_STATUS_ARB)
        self._checkOpenGLError()
        if not success:
            raise ShaderError(glGetInfoLogARB(shaderHandle))
        glAttachObjectARB(self._shaderProgramID, shaderHandle)
        self._checkOpenGLError()
        self._shaderObjectList.append(shaderHandle)
        
    def linkShaders(self):
        """
        Links compiled shader programs.
        """
        glLinkProgramARB(self._shaderProgramID)
        self._checkOpenGLError()
        success = glGetObjectParameterivARB(self._shaderProgramID, 
            GL_OBJECT_LINK_STATUS_ARB)
        if not success:
            raise ShaderError(glGetInfoLogARB(self._shaderProgramID))
        else:
            self._programReady = True
    
    def enable(self):
        """
        Activates shader programs.
        """
        if self._programReady:
            glUseProgramObjectARB(self._shaderProgramID)
            self._isEnabled=True
            self._checkOpenGLError()
        else:
            print "Shaders not compiled/linked properly, enable() failed"

    def disable(self):
        """
        De-activates shader programs.
        """
        glUseProgramObjectARB(0)
        self._isEnabled=False
        self._checkOpenGLError()

    def indexOfUniformVariable(self, variableName):
        """
        Finds the index of a uniform variable.
        """
        if not self._programReady:
            print "\nShaders not compiled/linked properly"
            result = -1
        else:
            result = glGetUniformLocationARB(self._shaderProgramID, variableName)
            self._checkOpenGLError()
        if result < 0:
            raise ShaderError('Variable "%s" not known to the shader' % (variableName))
        else:
            return result

    def indexOfVertexAttribute(self, attributeName):
        """
        Finds the index of an attribute variable.
        """
        if not self._programReady:
            print "\nShaders not compiled/linked properly"
            result = -1
        else:
            result = glGetAttribLocationARB(self._shaderProgramID, attributeName)
            self._checkOpenGLError()
        if result < 0:
            raise ShaderError('Attribute "%s" not known to the shader' % (attributeName))
        else:
            return result
    
    def isEnabled(self):
        return self._isEnabled

    def glUniform3f(self, variable_name, a, b, c):
        if self.isEnabled():
            index = self.indexOfUniformVariable(variable_name)
            glUniform3fARB(index, GLfloat(a), GLfloat(b), GLfloat(c))
        else:
            raise ShaderError("You must enable this shader to modify a uniform variable.")

    def glUniform1f(self, variable_name, a):
        if self.isEnabled():
            index = self.indexOfUniformVariable(variable_name)
            glUniform1fARB(index, GLfloat(a))
        else:
            raise ShaderError("You must enable this shader to modify a uniform variable.")
    def glUniform1i(self, variable_name, a):
        if self.isEnabled():
            index = self.indexOfUniformVariable(variable_name)
            glUniform1iARB(index, a)
        else:
            raise ShaderError("You must enable this shader to modify a uniform variable.")

# test routine
if __name__ == '__main__':
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)
    glutInitWindowSize(100, 100)
    glutCreateWindow("shaderProg Test")
    # Note: this will fail on many platforms where you must call
    # *after* you get an initialized glut context...
    Sp = ShaderProgram()
    Sp.addShader(GL_VERTEX_SHADER_ARB, "temperature.vert")
    Sp.addShader(GL_FRAGMENT_SHADER_ARB, "temperature.frag")
    Sp.linkShaders()
    print "Index of variable CoolestTemp: ", \
        Sp.indexOfUniformVariable("CoolestTemp")
    Sp.enable()
    print "Index of attribute VertexTemp: ", \
        Sp.indexOfVertexAttribute("VertexTemp")
    glVertexAttrib1fNV(Sp.indexOfVertexAttribute("VertexTemp"), 12.3)
    Sp.disable()
    Sp.reset()
    print 'OK'
