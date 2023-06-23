$input v_texcoord0

#include <../mrender.sh>

SAMPLER2D(s_Shadow,  0);
SAMPLER2D(s_Color,   1);

void main() 
{
	vec4 shadow = texture2D(s_Shadow, v_texcoord0);
	vec4 color = texture2D(s_Color, v_texcoord0);
	gl_FragColor = vec4(v_texcoord0, 0.0, 1.0);
	gl_FragColor = vec4(shadow.rg, color.ba);
	gl_FragColor = color;
}