#version 460 core

out vec4 FragColor;

in vec3 fragNormal;
uniform vec3 uColor;

void main()
{
    float intensity = dot(normalize(fragNormal), normalize(vec3(0.0, 0.0, 1.0)));
    intensity = clamp(intensity, 0.1, 1.0);
    FragColor = vec4(uColor * intensity, 1.0);
}
