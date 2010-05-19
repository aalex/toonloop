#include <GL/glew.h>
#include "shader.h"
#include <iostream>
#include <cstdlib>

// a simple vertex shader, identical to tester effect from v1.2
static const char *vertex_source = "\
  varying vec2 texcoord0;\
  varying vec2 texdim0;\
\
  void main(void)\
  {\
      gl_Position = ftransform();\
      texcoord0 = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);\
      texdim0 = vec2(abs(gl_TextureMatrix[0][0][0]), abs(gl_TextureMatrix[0][1][1]));\
  }\
 ";

        
// a simple fragment shader, identical to tester effect from v1.2
static const char *fragment_source = "\
uniform sampler2DRect image;\
varying vec2 texcoord0;\
varying vec2 texdim0;\
void main(void)\
{   \
	    vec3 color = texture2DRect(image, texcoord0).bgr;\
	    float input_alpha = gl_Color.a;\
	    gl_FragColor = vec4(color, input_alpha);\
}\
";

Shader::Shader()
{
    ;
}

void Shader::log(GLuint obj)
{
	GLint infologLength = 0, charsWritten = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 1) 
    {
		GLchar* infoLog = new GLchar [infologLength];
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		std::cout << infoLog << std::endl;
		delete infoLog;
	}
}


int Shader::compile_link()
{

    program_object_ = glCreateProgram(); // creating a program object
    vertex_shader_ = glCreateShader(GL_VERTEX_SHADER); // creating a vertex shader object
    fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER); // creating a fragment shader object

    glShaderSource(vertex_shader_, 1, &vertex_source, NULL); // assigning the vertex source
    glShaderSource(fragment_shader_, 1, &fragment_source, NULL); // assigning the fragment source
    log(program_object_); // verifies if all this is ok so far
    
    // compiling and attaching the vertex shader onto program
    glCompileShader(vertex_shader_);
    glAttachShader(program_object_, vertex_shader_); 
    log(program_object_); // verifies if all this is ok so far
    
    // compiling and attaching the fragment shader onto program
    glCompileShader(fragment_shader_);
    glAttachShader(program_object_, fragment_shader_); 
    log(program_object_); // verifies if all this is ok so far
    
    // Link the shaders into a complete GLSL program.
    glLinkProgram(program_object_);
    log(program_object_); // verifies if all this is ok so far
    
    // some extra code for checking if all this initialization is ok
    GLint prog_link_success;
    glGetObjectParameterivARB(program_object_, GL_OBJECT_LINK_STATUS_ARB, &prog_link_success);
    if (!prog_link_success) 
    {
        fprintf(stderr, "The shaders could not be linked\n");
        exit(1);
    }
    else 
        fprintf(stderr, "The shaders were linked\n");
}

GLuint Shader::get_program_object()
{
    return program_object_;
} 

