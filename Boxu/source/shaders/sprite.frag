#version 330

in vec2 fragTexCoord;

out vec4 outColor;

uniform sampler2D uTexture;
uniform vec4 color;
uniform int uCircleMask;

void main()
{
	if (uCircleMask != 0)
	{
		vec2 centered = fragTexCoord - vec2(0.5, 0.5);
		if (dot(centered, centered) > 0.25)
		{
			discard;
		}
	}
	outColor = color * texture(uTexture, fragTexCoord);
}