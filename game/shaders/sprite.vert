#version 330 core
// Placeholder for COMP710 sprite pipeline (Phase 1+)
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
uniform mat4 uWorldTransform;
uniform mat4 uViewProj;
void main()
{
    vTexCoord = aTexCoord;
    gl_Position = uViewProj * uWorldTransform * vec4(aPosition, 1.0);
}
