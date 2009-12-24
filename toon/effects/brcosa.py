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
# fragment shader for adjusting brightness, contrast and saturation
frag = """
#extension GL_ARB_texture_rectangle : enable
// user configurable variables (read-only here)
varying vec2 texcoord0;
varying vec2 texdim0;
// the texture
uniform sampler2DRect image;
// arguments
uniform vec3 avgluma;
uniform float saturation;
uniform float contrast;
uniform float brightness;
uniform float alpha;
// constants
const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
void main (void)
{
	vec3 texColor = texture2DRect(image, texcoord0).rgb;
	vec3 intensity = vec3(dot(texColor, LumCoeff));
	vec3 color = mix(intensity, texColor, saturation);
	color = mix(avgluma, color, contrast);
	color *= brightness;
	gl_FragColor = vec4(color, color.g*alpha);
}
"""
# ----------------------------  functions ----------------------------
class BrCoSaEffect(object):
    def __init__(self):
        self.program = None
        self.loaded = False
        self.enabled = False
        self.config = {
            'avgluma_r':0.5,
            'avgluma_g':0.5,
            'avgluma_b':0.5,
            'saturation':0.5,
            'contrast':0.5,
            'brightness':0.5,
            'alpha':0.5,
            'texture_id':0,
            }
        self.name = "brcosa"
        print("init %s" % (self.name))
        
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
            print("Set up effect %s" % (self.name))
            print(self.config)
    
    def pre_draw(self):
        if self.enabled and self.loaded:
            try:
                self.program.enable()
            except Exception, e: 
                print(e.message)
            self.program.glUniform1i("image", self.config["texture_id"])
            self.program.glUniform1f("saturation", self.config["saturation"])
            self.program.glUniform1f("contrast", self.config["contrast"])
            self.program.glUniform1f("brightness", self.config["brightness"])
            self.program.glUniform1f("alpha", self.config["alpha"])
            self.program.glUniform3f(
                "avgluma", 
                self.config['avgluma_r'], 
                self.config['avgluma_g'], 
                self.config['avgluma_b']
                )

    def post_draw(self):
        if self.enabled and self.loaded:
            try:
                self.program.disable()
            except Exception, e: 
                print e.message

def create_effect():
    return BrCoSaEffect()
