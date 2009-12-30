#!/usr/bin/env python
"""
GLSL shaders with SDL, OpenGL texture and Python
"""
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.error import NullFunctionError #TODO: port to all other shader.
from OpenGL.GLU import *
from rats.glsl import ShaderProgram
from rats.glsl import ShaderError
from toon import fx
from toon import optgroup

# globals
vert = """
/**
 * Vertex shader that does nothing
 */
// variables passed to the fragment shader
varying vec2 texcoord0;
varying vec2 texdim0;
void main()
{
    gl_Position = ftransform();
    texcoord0 = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
    texdim0 = vec2(abs(gl_TextureMatrix[0][0][0]), abs(gl_TextureMatrix[0][1][1]));
}
"""
frag = """
/**
 * Fragment shader for chroma-keying. 
 * 
 * (using a green or blue screen, or any background color)
 * 
 * Main thing is, make sure the texcoord's arent 
 * normalized (so they are in the range of [0..w, 0..h] )
 * 
 * All params are vec3 in the range [0.0, 1.0]
 * 
 * :param keying_color: The RGB keying color that will be made transparent.
 * :param thresh: The distance from the color for a pixel to disappear.
 * 
 * :author: Alexandre Quessy <alexandre@quessy.net> 2009
 * :license: GNU Public License version 3
 * 
 *  smoothstep with distance from target color
 *  genType smoothStep(genType edge0, genType edge1, genType x);
 *  The result will be zero if x <= edge0, 1 
 *  if x >= edge1 and performs smooth Hermite interpolation 
 *  between 0 and 1 when edge0 < x < edge1. 
 */

// user-configurable variables (read-only)
uniform vec3 keying_color;
uniform float thresh; // [0, 1.732]
uniform float slope; // [0, 1]
// the texture
uniform sampler2DRect image;
// data passed from vertex shader:
varying vec2 texcoord0;
varying vec2 texdim0;
void main(void)
{
    // sample from the texture 
    vec3 input_color = texture2DRect(image, texcoord0).rgb;
    float input_alpha = gl_Color.a;
    float d = abs(length(abs(keying_color.rgb - input_color.rgb)));
    float edge0 = thresh * (1.0 - slope); 
    float alpha = smoothstep(edge0, thresh, d);
    gl_FragColor = vec4(input_color, alpha * input_alpha); 
}
"""

class ChromaKeyOptions(optgroup.OptionsGroup):
    def __init__(self):
        self.target_color = [0.2, 0.0, 0.8]
        self.thresh = 0.8
        self.slope = 0.2
        self.texture_id = 0

class ChromaKeyEffect(fx.Effect):
    """
    Chroma keying filter using GLSL.
    """
    def __init__(self):
        fx.Effect.__init__(self)
        self.name = "chromakey"
        self.program = None
        self.options = ChromaKeyOptions()

    def setup(self):
        """
        Loads the plugin
        """
        global vert
        global frag
        try:
            self.program = ShaderProgram()
            self.program.add_shader_text(GL_VERTEX_SHADER_ARB, vert)
            self.program.add_shader_text(GL_FRAGMENT_SHADER_ARB, frag)
            self.program.linkShaders()
            self.loaded = True
        except NullFunctionError, e:
            print("Error: %s" % (e.message))
            print("Disabling the chromakey effect")
        except ShaderError, e:
            print("Error: %s" % (e.message))
            print("Disabling the chromakey effect")
        self.loaded = True
    
    def pre_draw(self):
        if self.loaded and self.enabled:
            try:
                self.program.enable()
            except Exception, e: 
                self.loaded = False
                print(e.message)
            else:
                self.program.glUniform1i("image", self.options.texture_id)
                self.program.glUniform3f("keying_color", *self.options.target_color)
                self.program.glUniform1f("thresh", self.options.thresh)
                self.program.glUniform1f("slope", self.options.slope)

    def post_draw(self):
        if self.loaded and self.enabled:
            try:
                self.program.disable()
            except Exception, e: 
                self.loaded = False
                print(e.message)

def create_effect():
    """
    Factory for the effect in this module.
    """
    return ChromaKeyEffect()
