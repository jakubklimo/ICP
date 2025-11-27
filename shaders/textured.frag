#version 460 core
in vec2 TexCoord;

uniform sampler2D tex0;

out vec4 FragColor;

void main()
{
    FragColor = texture(tex0, TexCoord);
}
