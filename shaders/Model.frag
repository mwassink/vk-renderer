#version 450


#define M_PI 3.1415926535897932384626433832795
#define MAX_LIGHTS 100
#define MAX_OBJECTS 500

struct Light {
  vec4 lightPos; //position in camera space
  vec4 lightColor;
  vec2 FN;
  float power;
  float shininess;

    
};

layout(set=0, binding=1) uniform lightingUniforms {
    Light lights[100];
};


layout(set=0, binding=2) uniform sampler2D roughnessMap[MAX_OBJECTS];
layout(set=0, binding=3) uniform sampler2D diffuseMap[MAX_OBJECTS];
layout(set=0, binding=4) uniform sampler2D normalMap[MAX_OBJECTS];
layout(set=0, binding=5) uniform sampler2D specularMap[MAX_OBJECTS];



layout(location=0) in vec4 posWorldSpace;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 n;
layout(location=3) in vec3 t;
layout(location=4) in vec4 b;

layout(location=0) out vec4 worldSpaceNormal;
layout(location=1) out vec3 diffuseColor;
layout(location=2) out vec3 specularColor;
layout(location=3) out vec3 f0Out;
layout(location=4) out float roughness;


layout (push_constant) uniform push_constants {
    int lightNum;
    int objNum;
    vec3 f0;
};


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


float getDepth(vec4 pIn) {
    float zSample = max(abs(pIn.x), max(abs(pIn.y), abs(pIn.z)));
    float normZ = (FN.x + FN.y)/(FN.x - FN.y) - (2*FN.x * FN.y)/(FN.x-FN.y)/zSample;
    return  (normZ + 1.0) * 0.5;
}

    
float fetchCoeff(vec4 posIn) {
    vec4 posNew = vec4(posIn.x, posIn.y, -posIn.z, getDepth(posIn));
    float s = texture(depthMaps[lightNum], posNew); // do not need to divide by w, just use direction vector to sample the cube map
    return s;   
}

void main(void) {
    

    vec4 normalRaw = texture(normalMaps[objNum], uvCoord);

    
    worldSpaceNormal = vec3(normalRaw.x * t, normalRaw.y * b, normalRaw.z * n);
    diffuseColor = texture(diffuseMap[objNum], uvCoord).xyz;
    specularColor = texture(specularMap[objNum], uvCoord).xyz;
    f0Out = f0;
    roughness = texture(roughnessMap[objNum], uvCoord).xyz;
    
}