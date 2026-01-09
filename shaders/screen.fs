#version 400
in vec2 vuv;
uniform sampler2D textureUniform;
out vec4 frag_colour;
void main( )
{
vec4 value = texture(textureUniform, vuv);
// frag_colour = vec4(vec3(value), 1.0);

// frag_colour = vec4(1.0, 0.0, 0.0, 1.0);
frag_colour = vec4(vec3(1.0 - texture(textureUniform,vuv.st)), 1.0);

}