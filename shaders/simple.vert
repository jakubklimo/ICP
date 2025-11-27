#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords; // Zùstává kvùli kompatibilitì formátu Vertex

out vec3 fragNormal;

uniform mat4 uModel; // Zpìt na pùvodní název, který používá C++ engine
uniform mat4 uMVP;   // Zpìt na pùvodní název, který používá C++ engine

void main()
{
    // Transformace normál pomocí inverzní transponované Model matice
    fragNormal = mat3(transpose(inverse(uModel))) * aNormal;
    
    // Transformace pozice do Clip space
    gl_Position = uMVP * vec4(aPos, 1.0);
}