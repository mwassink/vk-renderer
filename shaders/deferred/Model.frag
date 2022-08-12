#version 460


#define M_PI 3.1415926535897932384626433832795
#define MAX_LIGHTS 100
#define MAX_OBJECTS_GROUP 5

struct Light {
  vec4 lightPos; //position in camera space
  vec4 lightColor;
  vec2 FN;
  float power;
  float shininess;

    
};

layout(set=0, binding=1) uniform sampler2D roughnessMap;
layout(set=0, binding=2) uniform sampler2D diffuseMap;
layout(set=0, binding=3) uniform sampler2D normalMap;
layout(set=0, binding=4) uniform sampler2D specularMap;



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
layout(location=5) out vec4 pos;


layout (push_constant) uniform push_constants {
    int objNum;
    vec3 f0;
};

float getDepth(vec4 pIn) {
    float zSample = max(abs(pIn.x), max(abs(pIn.y), abs(pIn.z)));
    float normZ = (FN.x + FN.y)/(FN.x - FN.y) - (2*FN.x * FN.y)/(FN.x-FN.y)/zSample;
    return  (normZ + 1.0) * 0.5;
}


void main(void) {
    

    vec4 normalRaw = texture(normalMaps, uvCoord);

    
    worldSpaceNormal = vec3(normalRaw.x * t, normalRaw.y * b, normalRaw.z * n);
    diffuseColor = texture(diffuseMap, uvCoord).xyz;
    specularColor = texture(specularMap, uvCoord).xyz;
    f0Out = f0;
    roughness = texture(roughnessMap, uvCoord).xyz;
    
}