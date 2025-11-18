#version 400 core
out vec4 FragColor;
in vec3 fPos;
in vec3 fNor;
in vec2 uv;

uniform float ambientLightIntensity;
uniform vec3 ambientLightColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform vec3 lightPosition;

void main()
{
    vec3 objectColor = vec3(1.0, 0.0, 0.0);

    vec3 worldLightDirection = normalize(lightPosition - fPos);

    vec3 ambient = ambientLightIntensity * ambientLightColor;
    vec3 diffuse = max(dot(-worldLightDirection,fNor), 0.0) * lightColor;

//     float distance = distance (lightPosition - fPos);
//     float c2 = 0.3f;
//     float attenuation = 1.0/c2* distance;

    vec3 result = (ambient + diffuse) * objectColor;

   FragColor = vec4(result.x, result.y, result.z, 1);
}