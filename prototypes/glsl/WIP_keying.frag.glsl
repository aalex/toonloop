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
 * :param thresh: The distance from the color for the middle of the slope.
 * :param slope: 0.0 for a steep slope. 1.0 for a very gradual gradient.
 * 
 * :author: Alexandre Quessy <alexandre@quessy.net> 2009
 * :license: GNU Public License version 3
 */

// user-configurable variables (read-only)
uniform vec3 keying_color;
uniform vec3 thresh; // how close to this color a pixel disapears.
uniform float slope; // How fast pixels disappear if below thresh. 

// the texture
uniform sampler2DRect image;

// data passed from vertex shader:
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
	if (delta.r <= (thresh.r + slope) && 
        delta.g <= (thresh.g + slope) && 
        delta.b <= (thresh.b + slope))
	{
	   //output_alpha = 0.0;
       // using the average distance and the slope as a factor
       float avg_delta = ((delta.r + delta.g + delta.b) / 3.0);
       float avg_thresh = ((thresh.r + thresh.g + thresh.b) / 3.0);
       output_alpha = mix(0.0, 1.0, );
	}
    
    // the resulting color
    gl_FragColor = vec4(input_color, output_alpha) ; // * input_color.a); ???
}

