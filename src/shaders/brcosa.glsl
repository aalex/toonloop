/** 
 * fragment shader for adjusting brightness, contrast and saturation
 */
// the texture
uniform sampler2D image;
// arguments
uniform vec3 avgluma;
uniform float saturation;
uniform float contrast;
uniform float brightness;
uniform float alpha;
uniform float opacity; // how much efficient this brcosa is. reinject some of the orig. img.
// constants
const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);

void main (void)
{
    float input_alpha = gl_Color.a;
    vec3 texColor = vec3(texture2D(image, gl_TexCoord[0].st));
    vec3 intensity = vec3(dot(texColor, LumCoeff));
    vec3 color = mix(intensity, texColor, saturation);
    color = mix(avgluma, color, contrast);
    color *= brightness;
    color = mix(color, texColor, opacity); // reinject some of the original image in it.
    gl_FragColor = vec4(color, alpha * input_alpha);
}

