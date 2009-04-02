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

uniform vec3 keying_color;
uniform vec3 thresh; // how close to this color a pixels disapears.

uniform sampler2DRect image;

varying vec2 texcoord0;
varying vec2 texdim0;

void main(void)
{
    // sample from the texture 
    vec3 input_color = texture2DRect(image, texcoord0).rgb;
    float output_alpha = 1.0;
    
    // measure distance from keying_color
    vec3 delta = abs(input_color - keying_color);
	
	// for now, not visible if under threshold of proximity
	// TODO: mix() according the 3 factors of proximity.
	if (delta.r <= thresh.r && delta.g <= thresh.g && delta.b <= thresh.b)
	{
	   output_alpha = 0.0;
	}
	
    // if (color.g >= 0.5 && color.b <= 0.5 && color.r <= 0.5) 
    // {
    //     output_alpha = 0.0;
    // }
    // gl_FragColor = alpha * vec4(color, alpha);
    
    gl_FragColor = vec4(input_color, output_alpha); 
}
