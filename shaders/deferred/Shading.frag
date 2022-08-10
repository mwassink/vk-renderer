#version 460

struct Light {
  vec4 lightPos; //position in camera space
  vec4 lightColor;
  float power;
  float shininess;    
};


layout(set=0, binding=1) uniform lightingUniforms {
    Light lights[100];
    vec4 userPos;
};


layout(set=0, binding=2) uniform sampler2D f0tex;
layout(set=0, binding=3) uniform sampler2D roughnesstex;
layout(set=0, binding=4) uniform sampler2D diffusetex;
layout(set=0, binding=5) uniform sampler2D normaltex;
layout(set=0, binding=6) uniform sampler2D speculartex;
layout(set=0, binding=7) uniform sampler2D postex;

layout(location=0) in vec4 pos;
layout(location=1) in vec2 uv;


layout(location=0) out vec4 color;

vec3 CookTorrance(float roughness, vec3 n, vec3 l, vec3 h, vec3 f0, vec3 specularColor) {

    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;

    float ndoth = dot(n, h);
    float ndotv = dot(n, v);
    float ndotl = dot(n, l);
    float prod =  ( ndoth*ndoth*(alphaSquared-1) + 1);
    float dh = alphaSquared / (PI * prod * prod); // normal distribution

    float k = (roughness + 1) * (roughness + 1) * .125;
    float G1v = ndotv / (ndotv*(1-k) + k)
    float G1l = ndotl / (ndotl * (1-k) + k)
    float g = G1v * G1l; // masking

    float F = f0 + (1- f0)* pow(2, (-5.55 * vdoth - 6.98)*vdoth );

    return (dh * F * g * specularColor) / ( 4 * ndotl * ndotv);
    
}

layout (push_constant) uniform lightIndex {
    int lightNum;
};


void main(void) {

    vec3 specular = vec3(0, 0, 0);
    vec3 diffuse = vec3(0, 0, 0);

    vec3 pos = texture(postex, uv);
    vec3 l = lights[lightNum].lightPos.xyz - pos;
    float distanceSquared = dot(l, l);
    float power = lights[lightNum].power.xyz / (4 * PI * distanceSquared);
    vec3 diffColor = texture(diffusetex, uv) * power;

    
    float roughness = texture(roughnesstex, uv);
    vec3 n = texture(normaltex, uv);
    vec3 specularColor = texture(speculartex, uv);
    
    vec3 f0 = texture(f0tex, uv);
    vec3 v = userPos - pos;
    vec3 h = normalize(l + v);
    vec3 specular = CookTorrance(roughness, n, l , h, f0, specularColor);

    
    color = diffColor + specular;
}