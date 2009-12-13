#!/usr/bin/env python
"""
GLSL shaders with SDL, OpenGL texture and Python
"""
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
from rats.glsl import ShaderProgram
from rats.glsl import ShaderError
from toon import fx

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
    float d = abs(length(abs(keying_color.rgb - input_color.rgb)));
    float edge0 = thresh * (1.0 - slope); 
    float alpha = smoothstep(edge0, thresh, d);
    gl_FragColor = vec4(input_color, alpha); 
}
"""

class ChromaKeyEffect(fx.Effect):
    """
    Chroma keying filter using GLSL.
    """
    def __init__(self):
        self.name = "chromakey"
        self.is_on = False
        self.is_enabled = False
        self.program = None
        self.config = {
            'chromakey_r':0.2,
            'chromakey_g':0.8,
            'chromakey_b':0.0,
            'chromakey_thresh':0.8,
            'chromakey_slope':0.2,
            #'chromakey_on':False,
            #'chromakey_enabled':False,
            '_texture_id':0,
            }
        #print("init %s" % (self.name))
        fx.Effect.__init__(self)
    
    def update_config(self, config):
        for k in self.config.iterkeys():
            if config.has_key(k):
                self.config[k] = config[k]
        if config.has_key("chromakey_on"):
            self.is_on = config["chromakey_on"]
        #if config.has_key("chromakey_enabled"):
        #    self.is_on = config["chromakey_enabled"]

    def setup(self):
        """
        Loads the plugin
        """
        global vert
        global frag
        self.program = ShaderProgram()
        self.program.add_shader_text(GL_VERTEX_SHADER_ARB, vert)
        self.program.add_shader_text(GL_FRAGMENT_SHADER_ARB, frag)
        self.program.linkShaders()
        # will throw an error if there is a problem
        self.is_enabled = True
        #print("Set up effect %s" % (self.name))
        #print(self.config)
    
    def pre_draw(self):
        if self.is_enabled and self.is_on:
            try:
                self.program.enable()
            except Exception, e: 
                self.loaded = False
                print(e.message)
            else:
                self.program.glUniform1i("image", self.config["_texture_id"])
                # program.glUniform1f("alpha_result", 0.2)
                self.program.glUniform3f("keying_color", self.config['chromakey_r'], self.config['chromakey_g'], self.config['chromakey_b'])
                self.program.glUniform1f("thresh", self.config['chromakey_thresh'])
                self.program.glUniform1f("slope", self.config['chromakey_slope'])

    def post_draw(self):
        if self.is_enabled and self.is_on:
            try:
                self.program.disable()
            except Exception, e: 
                self.loaded = False
                print(e.message)

def get_effect():
    """
    Factory for the effect in this module.
    """
    return ChromaKeyEffect()
