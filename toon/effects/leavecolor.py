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

class LeaveColorOptions(optgroup.OptionsGroup):
    def __init__(self):
        self.target_color = [1.0, 0.0, 0.0]
        self.avgluma = [1.0, 1.0, 1.0]
        self.thresh = [0.5, 0.5, 0.5]
        self.leave_saturation = 1.0
        self.allover_saturation = 0.0
        self.contrast = 1.0
        self.brightness = 1.0
        self.alpha = 1.0
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
 * Leave color effect.
 * 
 * :param target_color: The RGB keying color that will be made transparent.
 * :param thresh: The distance from the color for a pixel to disappear.
 * 
 * :author: Alexandre Quessy <alexandre@quessy.net> 2009
 * :license: GNU Public License version 3
 */

//// user-configurable variables (read-only)
uniform vec3 target_color;
uniform vec3 thresh; // [0, 1.732]
uniform vec3 avgluma;
uniform float leave_saturation;
uniform float allover_saturation;
uniform float contrast;
uniform float brightness;
uniform float alpha;

// the texture
uniform sampler2DRect image;

// constants
const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
// data passed from vertex shader:
varying vec2 texcoord0;
varying vec2 texdim0;

void main(void)
{
    // begin applying brcosa
    vec3 input_color = texture2DRect(image, texcoord0).rgb;
	vec3 intensity = vec3(dot(input_color, LumCoeff));
	vec3 color = mix(intensity, input_color, allover_saturation);
    // check if close to target color
    float input_alpha = gl_Color.a;
    vec3 delta = abs(input_color - target_color);
    if (delta.r <= thresh.r && delta.g <= thresh.g && delta.b <= thresh.b)
    {
        color = mix(intensity, input_color, leave_saturation);
    }
    // finish applying brcosa
	color = mix(avgluma, color, contrast);
	color *= brightness;
	gl_FragColor = vec4(color, alpha * input_alpha);
}
"""

class LeaveColorEffect(fx.Effect):
    """
    Makes all the image desaturated expect target color.
    """
    def __init__(self):
        fx.Effect.__init__(self)
        self.program = None
        self.options = LeaveColorOptions()
        self.name = "leavecolor"
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
            self.program.glUniform3f("target_color", *self.options.target_color)
            self.program.glUniform3f("thresh", *self.options.thresh)
            self.program.glUniform3f("avgluma", *self.options.avgluma)
            self.program.glUniform1f("alpha", self.options.alpha)
            self.program.glUniform1f("leave_saturation", self.options.leave_saturation)
            self.program.glUniform1f("allover_saturation", self.options.allover_saturation)
            self.program.glUniform1f("contrast", self.options.contrast)
            self.program.glUniform1f("brightness", self.options.brightness)

    def post_draw(self):
        if self.enabled and self.loaded:
            try:
                self.program.disable()
            except Exception, e: 
                print e.message

def create_effect():
    return LeaveColorEffect()

