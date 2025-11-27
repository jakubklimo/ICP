#version 460 core

out vec4 FragColor;

in vec3 fragNormal;

uniform vec3 uColor;
uniform vec3 uLightDir;

void main()
{
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
    
    vec3 normal = normalize(fragNormal);
    
    float diff = max(dot(normal, lightDir), 0.1);

    FragColor = vec4(uColor * diff, 1.0);
}