$input v_texcoord0

#include <../mrender.sh>

SAMPLER2D(s_Shadow,  0);
SAMPLER2D(s_Scene,   1);

void main() 
{
	vec4 shadow = texture2D(s_Shadow, v_texcoord0);
	vec4 color = texture2D(s_Scene, v_texcoord0);
	gl_FragColor = vec4(color.rgb, shadow.a);
}