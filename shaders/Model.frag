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

layout(set=0, binding=2) uniform samplerCubeShadow depthMaps[MAX_LIGHTS];
layout(set=0, binding=3) uniform sampler2D roughnessMap[MAX_OBJECTS];
layout(set=0, binding=4) uniform sampler2D diffuseMap[MAX_OBJECTS];
layout(set=0, binding=5) uniform sampler2D normalMap[MAX_OBJECTS];
layout(set=0, binding=6) uniform sampler2D specularMap[MAX_OBJECTS];



layout(location=0) in vec4 posModel;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 lightDir;
layout(location=3) in vec3 eyeDir;
layout(location=4) in vec4 shadowCoord;

layout(location=0) out vec4 color;


layout (push_constant) uniform push_constants {
    int lightNum;
    int objNum;
    float f0;
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
    
    vec3 l = normalize(lightDir);
    vec3 v = normalize(eyeDir);
    vec4 normalRaw = texture(normalMaps[objNum], uvCoord);
    vec3 n = normalize(2.0*normalRaw.xyz - 1.0); // [0, 1] -> [-1, 1]
    vec3 diffColor = texture(diffuseMap[objNum], uvCoord).xyz;

    float lambertian = max(dot(n, l), 0.0f);
    vec3 h = normalize(n + v);
    float spec = pow(max(dot(n, h), 0.0f), shininess);
    
    float scaleQuad = 1.0f / (4*M_PI*distSquared);
    float lightIntensity = lightBrightness * scaleQuad;

    float s = fetchCoeff(shadowCoord);
    vec3 diffRefl = (lambertian* lightIntensity*s)*diffColor*lightColor;

    vec3 specRefl = s * spec * lightIntensity * lightColor * specularColor;
    vec3 ambient = ambientCoeff * diffColor;
    color = vec4(diffRefl + specRefl + ambient, 1.0f);
    
}