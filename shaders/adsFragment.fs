#version 400 core
out vec4 FragColor;
in vec3 fPos;
in vec3 fNor;
in vec2 uv;

uniform float ambientLightIntensity;
uniform vec3 ambientLightColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;

void main()
{
    vec3 objectColor = vec3(1.0, 0.0, 0.0);
    vec3 ambient = ambientLightIntensity * ambientLightColor;
    vec3 diffuse = max(dot(-lightDirection,fNor), 0.0) * lightColor;


    vec3 result = (ambient + diffuse) * objectColor;

   FragColor = vec4(result.x, result.y, result.z, 1);
}