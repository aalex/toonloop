#include <GL/glew.h>
#include <stdio.h>
#include <GL/glx.h>

class Shader 
{
    public:
        Shader();
        int compile_link();
        void log(GLuint id);
        GLuint get_program_object();
    private:
        GLuint   program_object_;  // a handler to the GLSL program used to update
        GLuint   vertex_shader_;   // a handler to vertex shader. This is used internally 
        GLuint   fragment_shader_; // a handler to fragment shader. This is used internally too

};

bool check_if_shaders_are_supported();

