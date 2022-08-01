#version 450


layout(set=0, binding=2) uniform sampler2D roughness;
layout(set=0, binding=3) uniform sampler2D diffuse;
layout(set=0, binding=4) uniform sampler2D normal;
layout(set=0, binding=5) uniform sampler2D specular;
layout(set=0, binding=1) uniform sampler2D f0;

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


void main(void) {
    
}