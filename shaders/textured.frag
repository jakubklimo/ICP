#version 460 core

out vec4 FragColor;

in vec3 fragNormal;
in vec2 texCoords;

uniform sampler2D uTexture;
uniform vec3 uLightDir;

void main()
{
    vec3 ambient = vec3(0.1); 
    vec3 normal = normalize(fragNormal);
    
    vec3 lightDir = normalize(-uLightDir); 
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.9);
    
    vec3 illumination = ambient + diffuse;
    
    vec4 texColor = texture(uTexture, texCoords);

    FragColor = texColor * vec4(illumination, 1.0);
}