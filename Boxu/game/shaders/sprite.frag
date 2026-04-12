#version 330

in vec2 fragTexCoord;

out vec4 outColor;

uniform sampler2D uTexture;
uniform vec4 color;

void main()
{
	outColor = color * texture(uTexture, fragTexCoord);
}