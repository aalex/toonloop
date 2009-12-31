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

class Options(optgroup.OptionsGroup):
    def __init__(self):
        self.alpha = 0.5
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
// test for alpha using fragment shaders.
#extension GL_ARB_texture_rectangle : enable
uniform float alpha;
// the texture
uniform sampler2DRect image;
// data passed from vertex shader:
varying vec2 texcoord0;
varying vec2 texdim0;
void main(void)
{
    vec3 color = texture2DRect(image, texcoord0).rgb;
    float input_alpha = gl_Color.a;
    gl_FragColor = vec4(color, alpha * input_alpha);
}
"""

class Effect(fx.Effect):
    """
    Test effect that doesn't do much, just alpha.
    """
    def __init__(self):
        fx.Effect.__init__(self)
        self.program = None
        self.options = Options()
        self.name = "tester"
        
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
            self.program.glUniform1f("alpha", self.options.alpha)

    def post_draw(self):
        if self.enabled and self.loaded:
            try:
                self.program.disable()
            except Exception, e: 
                print e.message

def create_effect():
    #return Effect()
    return None


