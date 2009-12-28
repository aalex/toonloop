#!/usr/bin/env python
"""
GLSL shaders with SDL, OpenGL texture and Python
"""
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.error import GLError
from OpenGL.GLU import *
from rats.glsl import ShaderProgram
from rats.glsl import ShaderError
from toon import fx
from toon import optgroup

# Vertex shader that does nothing
#
# :texcoord0: variable passed to the fragment shader
# :texdim0: variable passed to the fragment shader
vert = """
varying vec2 texcoord0;
varying vec2 texdim0;
void main()
{
    gl_Position = ftransform();
    texcoord0 = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
    texdim0 = vec2(abs(gl_TextureMatrix[0][0][0]), abs(gl_TextureMatrix[0][1][1]));
}
"""

# Simple fragment shader for boolean chroma-keying. 
# 
# Makes pixels of a texture transparent if proximity to keying_color is under threshold.
# Make sure to use rectangle textures. 
# :param keying_color: The RGB keying color that will be made transparent.
# :param thresh: The distance from the color for a pixel to disappear.
# :author: Alexandre Quessy <alexandre@quessy.net> 2009
# :license: GNU Public License version 3
frag = """
#extension GL_ARB_texture_rectangle : enable
// user configurable variables (read-only here)
uniform vec3 keying_color;
uniform vec3 thresh;
uniform float alpha_under_thresh;
// the texture
uniform sampler2DRect image;
// data passed from vertex shader:
varying vec2 texcoord0;
varying vec2 texdim0;

void main(void)
{
    vec3 input_color = texture2DRect(image, texcoord0).rgb;
    float input_alpha = gl_Color.a;
    float output_alpha = input_alpha;
    vec3 delta = abs(input_color - keying_color);
    if (delta.r <= thresh.r && delta.g <= thresh.g && delta.b <= thresh.b)
    {
        output_alpha = alpha_under_thresh;
    }
    gl_FragColor = vec4(input_color, output_alpha); 
}
"""
# ----------------------------  functions ----------------------------
class SimpleChromaOptions(optgroup.OptionsGroup):
    def __init__(self):
        self.color = [0.2, 0.8, 0.0]
        self.thresh = [0.8, 0.8, 0.8]
        self.texture_id = 0
        self.alpha_under_thresh = 0.1

class SimpleChromaEffect(fx.Effect):
    def __init__(self):
        fx.Effect.__init__(self)
        self.program = None
        self.options = SimpleChromaOptions()
        self.name = "simplechroma"
        #print("init %s" % (self.name))
        
    def setup(self):
        global vert
        global frag
        if True:
        #try:
            self.program = ShaderProgram()
            self.program.add_shader_text(GL_VERTEX_SHADER_ARB, vert)
            self.program.add_shader_text(GL_FRAGMENT_SHADER_ARB, frag)
            self.program.linkShaders()
        #except Exception, e: 
        #    print(e.message)
        #else:
            self.loaded = True
            #print(self.config)
    
    def pre_draw(self):
        if self.enabled and self.loaded:
            try:
                self.program.enable()
            except Exception, e: 
                print(e.message)
            self.program.glUniform1i("image", self.options.texture_id)
            self.program.glUniform3f(
                "keying_color", *self.options.color)
            self.program.glUniform3f(
                "thresh", *self.options.thresh)
            self.program.glUniform1f("alpha_under_thresh", self.options.alpha_under_thresh)
#self.config['alpha_under_thresh'])

    def post_draw(self):
        if self.enabled and self.loaded:
            try:
                self.program.disable()
            except Exception, e: 
                print e.message

def create_effect():
    return SimpleChromaEffect()
