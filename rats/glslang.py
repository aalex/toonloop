# 
# Class to simplify the incorporation of GLSL programs.
# 
# Copyright (C) 2007  "Peter Roesch" <Peter.Roesch@fh-augsburg.de>
#
# This code is licensed under the PyOpenGL License.
# Details are given in the file license.txt included in this distribution.


import sys
try:
	from OpenGL.GLUT import *
	from OpenGL.GL import *
	from OpenGL.GLU import *
	from OpenGL.GL.ARB.shader_objects import *
	from OpenGL.GL.ARB.fragment_shader import *
	from OpenGL.GL.ARB.vertex_shader import *
except:
	print 'Error importing GL / shaders'
	sys.exit()

class ShaderProgram ( object ):
	"""Manage GLSL programs."""
	def __init__( self ):
		self.__requiredExtensions = ["GL_ARB_fragment_shader",
                 "GL_ARB_vertex_shader",
                 "GL_ARB_shader_objects",
                 "GL_ARB_shading_language_100",
								 "GL_ARB_vertex_shader",
								 "GL_ARB_fragment_shader"]
		self.checkExtensions( self.__requiredExtensions )
		self.__shaderProgramID = glCreateProgramObjectARB()
		self.__checkOpenGLError()
		self.__programReady = False
		self.__isEnabled = False
		self.__shaderObjectList = []

	def checkExtensions( self, extensions ):
		"""Check if all extensions in a list are present."""
		for ext in extensions:
			if ( not ext ):
				print "Driver does not support ", ext
				sys.exit()

	def __checkOpenGLError( self ):
		"""Print OpenGL error message."""
		err = glGetError()
		if ( err != GL_NO_ERROR ):
			print 'GLERROR: ', gluErrorString( err )
			sys.exit()

	def reset( self ):
		"""Disable and remove all shader programs"""
		for shaderID in self.__shaderObjectList:
			glDetachObjectARB( self.__shaderProgramID, shaderID )
			glDeleteObjectARB( shaderID )
			self.__shaderObjectList.remove( shaderID )
			self.__checkOpenGLError( )
		glDeleteObjectARB( self.__shaderProgramID )
		self.__checkOpenGLError( )
		self.__shaderProgramID = glCreateProgramObjectARB()
		self.__checkOpenGLError( )
		self.__programReady = False

	def addShader( self, shaderType, fileName ):
		"""Read a shader program from a file.

		The program is load and compiled"""
		shaderHandle = glCreateShaderObjectARB( shaderType )
		self.__checkOpenGLError( )
		sourceString = open(fileName, 'r').read()
		glShaderSourceARB(shaderHandle, [sourceString] )
		self.__checkOpenGLError( )
		glCompileShaderARB( shaderHandle )
		success = glGetObjectParameterivARB( shaderHandle, 
			GL_OBJECT_COMPILE_STATUS_ARB)
		if (not success):
			print glGetInfoLogARB( shaderHandle )
			sys.exit( )
		glAttachObjectARB( self.__shaderProgramID, shaderHandle )
		self.__checkOpenGLError( )
		self.__shaderObjectList.append( shaderHandle )

	def linkShaders( self ):
		"""Link compiled shader programs."""
		glLinkProgramARB( self.__shaderProgramID )
		self.__checkOpenGLError( )
		success = glGetObjectParameterivARB( self.__shaderProgramID, 
			GL_OBJECT_LINK_STATUS_ARB )
		if (not success):
			print glGetInfoLogARB(self.__shaderProgramID)
			sys.exit()
		else:
			self.__programReady = True
	
	def enable( self ):
		"""Activate shader programs."""
		if self.__programReady:
			glUseProgramObjectARB( self.__shaderProgramID )
			self.__isEnabled=True
			self.__checkOpenGLError( )
		else:
			print "Shaders not compiled/linked properly, enable() failed"

	def disable( self ):
		"""De-activate shader programs."""
		glUseProgramObjectARB( 0 )
		self.__isEnabled=False
		self.__checkOpenGLError( )

	def indexOfUniformVariable( self, variableName ):
		"""Find the index of a uniform variable."""
		if not self.__programReady:
			print "\nShaders not compiled/linked properly"
			result = -1
		else:
			result = glGetUniformLocationARB( self.__shaderProgramID, variableName)
			self.__checkOpenGLError( )
		if result < 0:
			print 'Variable "%s" not known to the shader' % ( variableName )
			sys.exit( )
		else:
			return result

	def indexOfVertexAttribute( self, attributeName ):
		"""Find the index of an attribute variable."""
		if not self.__programReady:
			print "\nShaders not compiled/linked properly"
			result = -1
		else:
			result = glGetAttribLocationARB( self.__shaderProgramID, attributeName )
			self.__checkOpenGLError( )
		if result < 0:
			print 'Attribute "%s" not known to the shader' % ( attributeName )
			sys.exit( )
		else:
			return result
	
	def isEnabled( self ):
		return self.__isEnabled

# test routine
if __name__ == '__main__':
	glutInit(sys.argv)
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)
	glutInitWindowSize( 100,100 )
	glutCreateWindow("shaderProg Test")
	# Note: this will fail on many platforms where you must call
	# *after* you get an initialized glut context...
	Sp = ShaderProgram()
	Sp.addShader( GL_VERTEX_SHADER_ARB, "temperature.vert" )
	Sp.addShader( GL_FRAGMENT_SHADER_ARB, "temperature.frag" )
	Sp.linkShaders( )
	print "Index of variable CoolestTemp: ", \
		Sp.indexOfUniformVariable( "CoolestTemp" )
	Sp.enable( )
	print "Index of attribute VertexTemp: ", \
		Sp.indexOfVertexAttribute( "VertexTemp" )
	glVertexAttrib1fNV(Sp.indexOfVertexAttribute( "VertexTemp" ), 12.3)
	Sp.disable( )
	Sp.reset( )
	print 'OK'
