$input v_texcoord0

#include <../mrender.sh>
#include <../mrender-common.sh>
#include <light.sh>

SAMPLER2D(u_gdepth, 0);
SAMPLER2D(u_gdiffuse, 1);
SAMPLER2D(u_gnormal, 2);
SAMPLER2D(u_gspecular, 3);
SAMPLER2D(u_gposition, 4);
SAMPLER2D(u_shadowMap, 5);

const int NR_LIGHTS = 4;
uniform vec4 u_lightPosOuterR[NR_LIGHTS];
uniform vec4 u_lightRgbInnerR[NR_LIGHTS];
uniform mat4 u_mtx;

void main()
{
	vec3  normal      = decodeNormalUint(texture2D(u_gnormal, v_texcoord0).xyz);
	float deviceDepth = texture2D(u_gdepth, v_texcoord0).x;
	float depth       = toClipSpaceDepth(deviceDepth);

	vec3 clip = vec3(v_texcoord0 * 2.0 - 1.0, depth);
#if !BGFX_SHADER_LANGUAGE_GLSL
	clip.y = -clip.y;
#endif // !BGFX_SHADER_LANGUAGE_GLSL
	vec3 wpos = clipToWorld(u_mtx, clip);

	vec3 view = mul(u_view, vec4(wpos, 0.0) ).xyz;
	view = -normalize(view);
	
	vec3 lighting = vec3_splat(0.0);
	for (int i = 0; i < NR_LIGHTS; i++)
	{
		lighting += calcLight(wpos, normal, view, u_lightPosOuterR[i].xyz, u_lightPosOuterR[i].w, u_lightRgbInnerR[i].xyz, u_lightRgbInnerR[i].w);;
	}

    gl_FragColor = vec4(lighting, 1.0);
}
