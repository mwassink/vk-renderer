#version 450

layout(set=0, binding=0) uniform matrixUniforms {
  mat4 mvp; // -> world space -> camera space -> clip space
  mat4 mv; // model matrix (to world space)
};

layout(location=0) in vec4 vPosition;
layout(location=1) in vec2 tCoord;
layout(location=2) in vec3 normal;

layout(location=0) out vec4 pos;
layout(location=1) out vec2 uv;
layout(location=2) out vec3 nPos;


out gl_PerVertex {
  vec4 gl_Position;
};


void main(void) {
  gl_Position = mvp * vPosition;
  pos = mv * vPosition; // world space
  uv = tCoord;
  nPos = normalMatrix * normal;
}