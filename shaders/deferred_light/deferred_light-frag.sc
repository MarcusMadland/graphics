$input v_texcoord0

#include <../mrender.sh>

SAMPLER2D(u_gdiffuse, 0);
SAMPLER2D(u_gnormal, 1);
SAMPLER2D(u_gspecular, 2);
SAMPLER2D(u_gposition, 3);
SAMPLER2D(u_shadowMap, 4);

uniform vec4 u_lightDir;
uniform vec4 u_lightColor;
uniform mat4 u_shadowViewProj;

const int NR_LIGHTS = 4;
uniform vec4 u_lightPositions[NR_LIGHTS];
uniform vec4 u_lightColors[NR_LIGHTS];
uniform vec4 u_viewPos;


void main()
{
// Todo
	vec3 position = texture2D(u_gposition, v_texcoord0).rgb;
	vec3 normal = texture2D(u_gnormal, v_texcoord0).rgb;
	float specular = texture2D(u_gspecular, v_texcoord0).r;
	vec3 diffuse = texture2D(u_gdiffuse, v_texcoord0).rgb;

	vec3 lighting = diffuse * 0.1;
	vec3 viewDir = normalize(u_viewPos.rgb - position);
	for (int i = 0; i < NR_LIGHTS; i++)
	{
		vec3 lightColor = u_lightColors[i].rgb;
	
		// diffuse
		vec3 lightDir = normalize(u_lightPositions[i].rgb - position);
		vec3 lightDiffuse = max(dot(normal, lightDir), 0.0) * diffuse * lightColor;
	
		// specular
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
		vec3 lightSpecular = lightColor * spec * specular;
		
		lighting += lightDiffuse + lightSpecular;
	}

    gl_FragColor = vec4(lighting, 1.0);
}
