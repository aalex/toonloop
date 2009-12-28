#!/usr/bin/env python
"""
GLSL shaders with SDL, OpenGL texture and Python

To test with glslang-run : 
./scripts/glslang-run -f lumakey -d 1
telnet localhost 15555
b 0,0,0
x invert 1
x alpha_over 0
x alpha_under 1
"""
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.error import GLError
from OpenGL.GLU import *
from rats.glsl import ShaderProgram
from rats.glsl import ShaderError
from toon import fx
from toon import optgroup

class LumaKeyOptions(optgroup.OptionsGroup):
    def __init__(self):
        self.avgluma = [1.0, 1.0, 1.0]
        self.luma_gate = 0.3
        self.saturation = 1.0
        self.contrast = 1.0
        self.invert = 1 # 0 or 1
        self.brightness = 1.0
        self.alpha_under = 1.0
        self.alpha_over = 0.0
        self.texture_id = 0

# ---------------------------- glsl vertex shader ----------
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

# ---------------------------- glsl fragment shader ----------
frag = """
#extension GL_ARB_texture_rectangle : enable
/**
 * Binary luma keying.
 * 
 * :author: Alexandre Quessy <alexandre@quessy.net> 2009
 * :license: GNU Public License version 3
 */

//// user-configurable variables (read-only)
uniform float luma_gate; // [0., 1.]
uniform vec3 avgluma;
uniform float saturation;
uniform float contrast;
uniform int invert;
uniform float brightness;
uniform float alpha_under;
uniform float alpha_over;

// the texture
uniform sampler2DRect image;

// constants
const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
const vec3 unity = vec3(1.0, 1.0, 1.0);
// data passed from vertex shader:
varying vec2 texcoord0;
varying vec2 texdim0;

void main(void)
{
    // begin applying brcosa
    vec3 input_color = texture2DRect(image, texcoord0).rgb;
	vec3 intensity = vec3(dot(input_color, LumCoeff));
	vec3 color = mix(intensity, input_color, saturation);
    float alpha = alpha_under;
    // check if close to target luminosity
    float input_alpha = gl_Color.a;
    if (intensity[0] >= luma_gate)
    {
        alpha = alpha_over;
    }
    // finish applying brcosa
	color = mix(avgluma, color, contrast);
    if (invert == 1) 
    {
        color = unity - color;
    }
	color *= brightness;
	gl_FragColor = vec4(color, alpha * input_alpha);
}
"""

class LumaKeyEffect(fx.Effect):
    """
    Makes all the image desaturated expect target color.
    """
    def __init__(self):
        fx.Effect.__init__(self)
        self.program = None
        self.options = LumaKeyOptions()
        self.name = "lumakey"
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
            #print("Set up effect %s" % (self.name))
            #print(self.config)
    
    def pre_draw(self):
        if self.enabled and self.loaded:
            try:
                self.program.enable()
            except Exception, e: 
                print(e.message)
            self.program.glUniform1i("image", self.options.texture_id)
            self.program.glUniform1f("luma_gate", self.options.luma_gate)
            self.program.glUniform3f("avgluma", *self.options.avgluma)
            self.program.glUniform1f("alpha_over", self.options.alpha_over)
            self.program.glUniform1f("alpha_under", self.options.alpha_under)
            self.program.glUniform1f("saturation", self.options.saturation)
            self.program.glUniform1f("contrast", self.options.contrast)
            self.program.glUniform1f("brightness", self.options.brightness)
            self.program.glUniform1i("invert", self.options.invert)

    def post_draw(self):
        if self.enabled and self.loaded:
            try:
                self.program.disable()
            except Exception, e: 
                print e.message

def create_effect():
    return LumaKeyEffect()

