/*
 * Toonloop
 *
 * Copyright 2010 Alexandre Quessy
 * <alexandre@quessy.net>
 * http://www.toonloop.com
 *
 * Toonloop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toonloop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the gnu general public license
 * along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <GL/glew.h>
#include <GL/glx.h>
#include <stdio.h>

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

