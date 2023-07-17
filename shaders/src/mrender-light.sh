#ifndef __MRENDERLIGHT_SH__
#define __MRENDERLIGHT_SH__

#include <../mrender.sh>
#include <../mrender-common.sh>

SAMPLER2D(u_gnormal, 0);
SAMPLER2D(u_gdepth,  1);
SAMPLER2D(u_gposition,  2);

uniform vec4 u_lightPositionRange; // rgb = position, a = range (radius)
uniform vec4 u_lightColorIntensity; // rgb = color, a = intensity

vec2 blinn(vec3 _lightDir, vec3 _normal, vec3 _viewDir)
{
	float ndotl = dot(_normal, _lightDir);
	vec3 reflected = _lightDir - 2.0*ndotl*_normal; // reflect(_lightDir, _normal);
	float rdotv = dot(reflected, _viewDir);
	return vec2(ndotl, rdotv);
}

float fresnel(float _ndotl, float _bias, float _pow)
{
	float facing = (1.0 - _ndotl);
	return max(_bias + (1.0 - _bias) * pow(facing, _pow), 0.0);
}

vec4 lit(float _ndotl, float _rdotv, float _m)
{
	float diff = max(0.0, _ndotl);
	float spec = step(0.0, _ndotl) * max(0.0, _rdotv * _m);
	return vec4(1.0, diff, spec, 1.0);
}

vec4 powRgba(vec4 _rgba, float _pow)
{
	vec4 result;
	result.xyz = pow(_rgba.xyz, vec3_splat(_pow) );
	result.w = _rgba.w;
	return result;
}

vec3 calcLight(vec3 _wpos, vec3 _normal, vec3 _view, vec3 _lightPos, float _lightRadius, vec3 _lightRgb, float _lightInner)
{
	vec3 lp = _lightPos - _wpos;
	float attn = 1.0 - smoothstep(_lightInner, 1.0, length(lp) / _lightRadius);
	vec3 lightDir = normalize(lp);
	vec2 bln = blinn(lightDir, _normal, _view);
	vec4 lc = lit(bln.x, bln.y, 1.0);
	vec3 rgb = _lightRgb * saturate(lc.y) * attn;
	return rgb;
}

vec4 calcLight(vec2 texcoord0)
{
	vec3 normal   = decodeNormalUint(texture2D(u_gnormal, texcoord0).rgb);
	vec3 position = texture2D(u_gposition, texcoord0).rgb;
	float alpha = texture2D(u_gposition, texcoord0).a;

	vec3 lightPosition = u_lightPositionRange.xyz;
	vec3 lightDir = normalize(lightPosition - position);
	float lightDistance = distance(lightPosition, position);

	float range = u_lightPositionRange.w;
    float attenuation = smoothstep(range, 0.0, lightDistance / range);
	
	vec3 lighting = max(dot(normal, lightDir), 0.0) * u_lightColorIntensity.rgb * u_lightColorIntensity.a * attenuation;
	//vec3 lighting = calcLight(position, normal, lightDir, lightPosition, u_lightPositionRange.a, u_lightColorIntensity.rgb, u_lightColorIntensity.a);
	return vec4(lighting.rgb, alpha);
}

#endif // __MRENDERLIGHT_SH__