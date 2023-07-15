#ifndef __MRENDERLIGHT_SH__
#define __MRENDERLIGHT_SH__

#include <../mrender.sh>
#include <../mrender-common.sh>

SAMPLER2D(u_gnormal, 0);
SAMPLER2D(u_gdepth,  1);
SAMPLER2D(u_gposition,  2);

uniform vec4 u_lightPositionRange; // rgb = position, a = range (radius)
uniform vec4 u_lightColorIntensity; // rgb = color, a = intensity

vec4 calcLight(vec2 texcoord0)
{
	vec3 normal   = decodeNormalUint(texture2D(u_gnormal, texcoord0).rgb);
	vec3 position = texture2D(u_gposition, texcoord0).rgb;

	vec3 lightPosition = u_lightPositionRange.xyz;
	vec3 lightDir = normalize(lightPosition - position);
	float lightDistance = distance(lightPosition, position);

	float range = u_lightPositionRange.w;
    float attenuation = smoothstep(range, 0.0, lightDistance / range);
	
	vec3 lighting = max(dot(normal, lightDir), 0.0) * u_lightColorIntensity.rgb * u_lightColorIntensity.a * attenuation;

	return vec4(lighting.rgb, 1.0);
}

#endif // __MRENDERLIGHT_SH__