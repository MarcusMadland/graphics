$input v_wpos, v_view, v_normal, v_tangent, v_bitangent, v_texcoord0

#include <../mrender.sh>

SAMPLER2D(u_albedo, 1);
SAMPLER2D(u_normal, 2);
SAMPLER2D(u_specular, 3);

void main() 
{
	vec4 albedoTex   = texture2D(u_albedo, v_texcoord0);
	vec4 normalTex   = texture2D(u_normal, v_texcoord0);
	vec4 specularTex = texture2D(u_specular, v_texcoord0);

	gl_FragData[0] = albedoTex;
	gl_FragData[1] = normalTex;
	gl_FragData[2] = specularTex;
}