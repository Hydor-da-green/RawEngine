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
uniform vec3 cameraPosition;

uniform sampler2D uvGridText;

void main()
{

    vec4 textColor = texture (uvGridText, uv);
    vec3 objectColor = vec3(1.0, 0.0, 0.0);

    vec3 worldLightDirection = normalize(lightPosition - fPos);

    vec3 ambient = ambientLightIntensity * ambientLightColor;
    vec3 diffuse = max(dot(worldLightDirection,fNor), 0.0) * lightColor;

    float distance = length(lightPosition - fPos);
    float c1 = 1.0f;
    float c2 = 0.02f;
    float c3 = 0.0f;
    float attenuation = 1.0/(c1 + c2 * distance + c3 * distance * distance);

    float s = 256;
    vec3 specularColor = vec3(1.0f, 1.0f, 1.0f);

    vec3 i = normalize (fPos - lightPosition);
    vec3 n = fNor;
    vec3 r = reflect(i, n);
    vec3 v = normalize( cameraPosition - fPos);
    vec3 specular = pow(max(dot(r, v), 0.0),s) *specularColor;
    vec3 color = (ambient + diffuse + specular) * textColor.rgb; //diffuse color


//     float distance = distance (lightPosition - fPos);
//     float c2 = 0.3f;
//     float attenuation = 1.0/c2* distance;

    vec3 result = (ambient + diffuse + specular) * textColor.rgb * attenuation;

   FragColor = vec4(result.x, result.y, result.z, 1);
   //vec4 textColor = texture (uvGridText, uv);
//    FragColor = vec4(textColor.x, textColor.y, textColor.z, 1);
}