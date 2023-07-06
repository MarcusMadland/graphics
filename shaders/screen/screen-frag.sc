$input v_texcoord0

#include <../mrender.sh>

SAMPLER2D(u_shadowMap,  0);
SAMPLER2D(u_color,   1);

void main() 
{
	vec4 color = texture2D(u_color, v_texcoord0);

	gl_FragColor = vec4(color.rgb, 1.0);
}