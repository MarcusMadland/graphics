$input v_texcoord0

#include <../mrender.sh>

SAMPLER2D(u_gdiffuse, 0);
SAMPLER2D(u_light, 1);

void main()
{

    vec3 diffuse = texture2D(u_gdiffuse, v_texcoord0).rgb;
    vec3 lighting = texture2D(u_light, v_texcoord0).rgb;
    
    vec3 finalColor = diffuse * lighting;
    
    gl_FragColor = vec4(finalColor, 1.0);
}
