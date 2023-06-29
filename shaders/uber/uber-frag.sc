$input v_wpos, v_view, v_normal, v_tangent, v_bitangent, v_texcoord0

#include <../mrender.sh>

SAMPLER2D(u_albedo, 0);
SAMPLER2D(u_normal, 1);
SAMPLER2D(u_specular, 2);

uniform vec4 u_albedoColor;
uniform vec4 u_normalColor;
uniform vec4 u_specularColor;

void main() 
{
	vec4 albedoTex   = u_albedoColor.a   > 0.0 ? u_albedoColor   : texture2D(u_albedo, v_texcoord0);
	vec4 normalTex   = u_normalColor.a   > 0.0 ? u_normalColor   : texture2D(u_normal, v_texcoord0);
	vec4 specularTex = u_specularColor.a > 0.0 ? u_specularColor : texture2D(u_specular, v_texcoord0);

	gl_FragData[0] = albedoTex;
	gl_FragData[1] = normalTex;
	gl_FragData[2] = specularTex;
}