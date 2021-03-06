#version 450


#define M_PI 3.1415926535897932384626433832795
#define MAX_LIGHTS 100

struct Light {
  vec4 lightPos; //position in camera space
  vec4 lightColor;
  float power;
  float shininess;
    
};

layout(set=0, binding=1) uniform lightingUniforms {
    Light lights[100];
};

layout(set=0, binding=2) uniform sampler2D tex;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 nPos;
layout (location = 0) out vec4 color;

layout (push_constant) uniform push_constants {
    int lightNum;
};


void main() {

  vec4 lightPos = lights[lightNum].lightPos;
  vec4 lightColor = lights[lightNum].lightColor;
  float power = lights[lightNum].power;
  float shininess = lights[lightNum].shininess;

  vec3 l = normalize(lightPos.xyz - pos.xyz);
  vec3 n = normalize(nPos);
  vec3 v = normalize(pos.xyz);

  float ndotl = max(dot(n, l), 0.0);
  vec3 h = normalize(n + v);

  float spec = pow(max(dot(n, h), 0.0f), shininess);
  float attenuation = 1.0f / (4 * M_PI*dot(lightPos - pos, lightPos - pos));
  float intensity = power * attenuation;

  vec3 diffRefl = (ndotl / M_PI * intensity) * texture(tex, uv).xyz;
  vec3 specRefl = spec * intensity * lightColor.xyz;
  //color = vec4(diffRefl + specRefl, 1.0f);
  color = texture(tex, uv);

}