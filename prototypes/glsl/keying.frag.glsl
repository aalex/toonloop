/**
 * Fragment shader for keying
 * 
 * user-configurable variables (read-only)
 * main thing is, make sure the texcoord's arnt normalized (so they are in the range of [0..w, 0..h] )
 */
// TODO : uniform float thresh;
/*
uniform float green_gt;
uniform float red_gt;
uniform float blue_gt; 
*/

uniform sampler2DRect image;

varying vec2 texcoord0;
varying vec2 texdim0;

void main(void)
{
    vec3 color = texture2DRect(image, texcoord0).rgb;
    float alpha = 1.0;
    
    if (color.g >= 0.5 && color.b <= 0.5 && color.r <= 0.5) 
    {
        alpha = 0.0;
    }
    //gl_FragColor = vec4(color.r, color.g, 1.0, alpha);
    gl_FragColor = alpha * vec4(color, alpha); 
}
