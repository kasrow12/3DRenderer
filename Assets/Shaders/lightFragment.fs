#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 viewPos;
uniform vec3 skyColor;
uniform float fogDistance;
uniform vec3 lightColor;

vec3 CalcFog(vec3 color);

void main()
{
    vec3 result = CalcFog(lightColor);

    FragColor = vec4(result, 1.0);
}

vec3 CalcFog(vec3 color)
{
    float distance = length(FragPos - viewPos);
    float fogFactor = (fogDistance - distance) / fogDistance;
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    return mix(skyColor, color, fogFactor);
}