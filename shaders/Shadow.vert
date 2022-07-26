#version 450

uniform mat4 modelLightProjection;

out gl_PerVertex {
  vec4 gl_Position;
};
layout (location = 0) in vec4 pos;
void main(void) {
    gl_Position = modelLightProjection * pos;
}