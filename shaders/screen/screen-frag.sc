$input v_texcoord0

#include <../mrender.sh>

SAMPLER2D(u_shadowMap,  0);
SAMPLER2D(u_color,   1);

void main() 
{
	vec4 shadow = texture2D(u_shadowMap, v_texcoord0);
	vec4 color = texture2D(u_color, v_texcoord0);
	gl_FragColor = vec4(v_texcoord0, 0.0, 1.0);
	gl_FragColor = vec4(shadow.rg, color.ba);
	gl_FragColor = vec4(color.rgb * 1.0, shadow.a);
}