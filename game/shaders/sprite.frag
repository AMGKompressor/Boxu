#version 330 core
// Placeholder for COMP710 sprite pipeline (Phase 1+)
in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D uTexture;
uniform vec4 uColourTint;
void main()
{
    vec4 tex = texture(uTexture, vTexCoord);
    FragColor = tex * uColourTint;
}
