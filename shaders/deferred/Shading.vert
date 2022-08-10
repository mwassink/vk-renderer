#version 460


#define M_PI 3.1415926535897932384626433832795
#define MAX_LIGHTS 100
#define MAX_OBJECTS 500

layout(set=0, binding=0) uniform matrixUniforms {
    mat4 mvp;
};

struct Light {
  vec4 lightPos; //position in camera space
  vec4 lightColor;
  float power;
  float shininess;    
};

layout(location=0) in vec4 pos;
layout(location=1) in vec2 uv;

void main(void) {
    gl_Position = mvp * pos;
    uv = gl_Position.xy / gl_Position.w;
}
