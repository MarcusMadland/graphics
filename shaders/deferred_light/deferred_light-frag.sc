$input v_texcoord0

#include <../mrender.sh>

SAMPLER2D(u_gdiffuse, 0);
SAMPLER2D(u_gnormal, 1);
SAMPLER2D(u_gspecular, 2);
SAMPLER2D(u_shadowMap, 3);

uniform vec4 u_lightDir;
uniform vec4 u_lightColor;
uniform mat4 u_shadowViewProj;

void main()
{
// Todo
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
