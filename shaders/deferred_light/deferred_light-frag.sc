$input v_texcoord0

#include <../mrender.sh>
#include <../mrender-common.sh>
#include <light.sh>

SAMPLER2D(u_gdiffuse, 0);
SAMPLER2D(u_gnormal, 1);
SAMPLER2D(u_gspecular, 2);
SAMPLER2D(u_gposition, 3);
SAMPLER2D(u_shadowMap, 4);

const int NR_LIGHTS = 4;
uniform vec4 u_lightPosOuterR[NR_LIGHTS];
uniform vec4 u_lightRgbInnerR[NR_LIGHTS];
uniform mat4 u_mtx;

void main()
{
	vec3 wpos = texture2D(u_gposition, v_texcoord0).rgb;
	vec4 normal4 = texture2D(u_gnormal, v_texcoord0);
	vec3 normal = normal4.a > 0.0 ? decodeNormalUint(normal4.rgb) : vec3_splat(0.0);
	
	vec3 view = mul(u_view, vec4(wpos, 0.0)).xyz;
	view = -normalize(view);
	
	vec3 lighting = vec3_splat(0.0);
	for (int i = 0; i < NR_LIGHTS; i++)
	{
		lighting += calcLight(wpos, normal, view, u_lightPosOuterR[i].xyz, u_lightPosOuterR[i].w, u_lightRgbInnerR[i].xyz, u_lightRgbInnerR[i].w);;
	}

    gl_FragColor = vec4(lighting, 1.0);
}
