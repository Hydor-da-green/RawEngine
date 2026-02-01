#version 400
in vec2 vuv;
uniform sampler2D textureUniform;
uniform bool framebuffer_is_active;
uniform float frameBufferEffect;
out vec4 frag_colour;
void main( )
{

if(frameBufferEffect==0){
vec4 value = texture(textureUniform, vuv);
// frag_colour = vec4(vec3(value), 1.0);
frag_colour = vec4(vec3(1.0 - texture(textureUniform,vuv.st)), 1.0);
}
if(frameBufferEffect==1){
//frag_colour = vec4(1.0, 0.0, 0.0, 1.0);
    float average = (texture(textureUniform, vuv).r + texture(textureUniform, vuv).g + texture(textureUniform, vuv).b) / 3.0;
//     frag_colour = vec4(average, average, average, 1.0);
    float offset = 0.003;
    vec2 offsets[9] = vec2[ ](
    vec2(-offset, offset), vec2(0.0f, offset), vec2(offset, offset),
    vec2(-offset, 0.0f), vec2(0.0f, 0.0f), vec2( offset, 0.0f),
    vec2(-offset, -offset), vec2(0.0f, -offset), vec2( offset, -offset)
    );
    float kernel[9] = float[ ](
    1.0, 1.0, 1.0,
    1.0, -5.9, 1.0,
    1.0, 1.0, 1.0
    );
    vec4 sampleTex[9];
    vec4 col = vec4(0.0);
    for(int i = 0; i < 9; i++)
    {
    col += texture(textureUniform, vuv.st + offsets[i]) * kernel[i] * (average/0.5f) * vec4(0.5, 0.0, 1.0, 0.5);
    }
    col.a = 1.0;
    frag_colour = col;

}






}