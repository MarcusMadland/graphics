$input v_wpos, v_normal, v_tangent, v_bitangent, v_texcoord0

#include <../mrender.sh>
#include <../mrender-common.sh>

SAMPLER2D(u_albedo, 0);
SAMPLER2D(u_normal, 1);
SAMPLER2D(u_specular, 2);

uniform vec4 u_albedoColor;
uniform vec4 u_normalColor;
uniform vec4 u_specularColor;

void main() 
{
	vec4 albedoCol   = u_albedoColor.a   > 0.0 ? u_albedoColor   : texture2D(u_albedo, v_texcoord0);
	vec4 normalCol   = u_normalColor.a   > 0.0 ? u_normalColor   : texture2D(u_normal, v_texcoord0);
	vec4 specularCol = u_specularColor.a > 0.0 ? u_specularColor : texture2D(u_specular, v_texcoord0);

	vec3 normal;
	normal.xy = normalCol.xy * 2.0 - 1.0;
	normal.z  = sqrt(1.0 - dot(normal.xy, normal.xy) );

	mat3 tbn = mat3(
				normalize(v_tangent),
				normalize(v_bitangent),
				normalize(v_normal)
				);

	normal = normalize(mul(tbn, normal) );

	vec3 wnormal = normalize(mul(u_invView, vec4(normal, 0.0) ).xyz);

	gl_FragData[0] = vec4(albedoCol.rgb, 1.0);
	gl_FragData[1] = vec4(encodeNormalUint(wnormal), 1.0);
	gl_FragData[2] = vec4(0.0, 0.0, 0.0, 1.0); // not in use
	gl_FragData[3] = vec4(v_wpos, 1.0);
}