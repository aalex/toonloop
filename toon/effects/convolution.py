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
// Convolution shader.
varying vec2 texcoord0;
varying vec2 texdim0;
// weights
uniform vec3 weight_row_0;
uniform vec3 weight_row_1;
uniform vec3 weight_row_2;

// sum of the weights.
uniform float sum;
// size of the kernel.
uniform float width;
uniform float height;
// texture
uniform sampler2DRect tex0;

void main()
{
    float input_alpha = gl_Color.a;
    vec2 op = texcoord0;
    vec2 ox = vec2(width, 0);
    vec2 oy = vec2(0, height);

    vec2 pos = op - oy;
    vec4 c00 = texture2DRect(tex0, pos - ox);
    vec4 c01 = texture2DRect(tex0, pos);
    vec4 c02 = texture2DRect(tex0, pos + ox);

    pos = op;
    vec4 c10 = texture2DRect(tex0, pos - ox);
    vec4 c11 = texture2DRect(tex0, pos);
    vec4 c12 = texture2DRect(tex0, pos + ox);

    pos = op + oy;
    vec4 c20 = texture2DRect(tex0, pos - ox);
    vec4 c21 = texture2DRect(tex0, pos);
    vec4 c22 = texture2DRect(tex0, pos + ox);
    
    vec3 output = c00 * weight_row_0[0];
    output += c01 * weight_row_0[1];
    output += c02 * weight_row_0[2];
    output += c10 * weight_row_1[0];
    output += c11 * weight_row_1[1];
    output += c12 * weight_row_1[2];
    output += c20 * weight_row_2[0];
    output += c21 * weight_row_2[1];
    output += c22 * weight_row_2[2];
    output /= sum;
    gl_FragColor = vec4(output.rgb, input_alpha);
}
"""

class Options(optgroup.OptionsGroup):
    def __init__(self):
        # edge detection with the laplacian operator:
        self.weight_row_0 = [0.0, 1.0, 0.0]
        self.weight_row_1 = [1.0, -4.0, 1.0]
        self.weight_row_2 = [0.0, 1.0, 0.0]
        self.sum = 0.0
        self.width = 2.0
        self.height = 2.0
        self.image = 0 # texture unit, not id !
        
        # ---- other example:
        # smoothing:
        #self.weight_row_0 = [1.0, 1.0, 1.0]
        #self.weight_row_1 = [1.0, 1.0, 1.0]
        #self.weight_row_2 = [1.0, 1.0, 1.0]
        #self.sum = 9.0

class Effect(fx.Effect):
    """
    Chroma keying filter using GLSL.
    """
    def __init__(self):
        fx.Effect.__init__(self)
        self.name = "convolution"
        self.program = None
        self.options = Options()

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
    
    def pre_draw(self):
        if self.loaded and self.enabled:
            try:
                self.program.enable()
            except Exception, e: 
                self.loaded = False
                print(e.message)
            else:
                self.program.glUniform1i("tex0", self.options.image)
                self.program.glUniform1f("width", self.options.width)
                self.program.glUniform1f("height", self.options.height)
                self.program.glUniform3f("weight_row_0", *self.options.weight_row_0)
                self.program.glUniform3f("weight_row_1", *self.options.weight_row_1)
                self.program.glUniform3f("weight_row_2", *self.options.weight_row_2)
                s = 0
                for l in [self.options.weight_row_0, self.options.weight_row_1,  self.options.weight_row_2]:
                    for i in l:
                        s += i
                self.options.sum = s
                self.program.glUniform1f("sum", self.options.sum)

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
    return Effect()

