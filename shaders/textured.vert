#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 fragNormal;
out vec2 texCoords;

uniform mat4 uModel;
uniform mat4 uMVP;

void main()
{
    fragNormal = mat3(transpose(inverse(uModel))) * aNormal;
    
    gl_Position = uMVP * vec4(aPos, 1.0);

    texCoords = aTexCoords;
}