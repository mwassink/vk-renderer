#version 450

#define MAX_LIGHTS 100
#define MAX_OBJECTS 500

struct Matrices {
    mat4 modelViewProjection;
    mat4 modelView;
    mat3 normalMatrix;
};

struct LightTransforms {
    mat4 modelLightMatrix;
    vec3 lightCameraSpace;
};



layout(set=0, binding=0) uniform matrixUniforms {
    Matrices matrices;
};

layout(set=0, binding=1) uniform lightUniforms {
    LightTransforms lightTransforms[100];
};


layout(location=0) in vec4 vPosition;
layout(location=1) in vec2 tCoord;
layout(location=2) in vec3 normal;

layout(location=0) out vec4 posModel;
layout(location=1) out vec2 uv;
layout(location=2) out vec3 lightDir;
layout(location=3) out vec3 eyeDir;
layout(location=4) out vec4 shadowCoord;




out gl_PerVertex {
    vec4 gl_Position;
}

void main(void) {
    vec3 n = normalize(normalMatrix * normal);
    vec3 t = normalize(normalMatrix * tangent);
    vec3 b = cross(n, t);
    b *= handedness;

    vec3 eyePos = (modelView * pos).xyz; // pos of pt in camera space. confusing...
    vec3 diff = lightCameraSpace - eyePos; // from pos to light
    lightDir = vec3(dot(diff, t), dot(diff, b), dot(diff, n) );
    lightDir = normalize(lightDir);

    eyeDir = vec3(dot(-eyePos, t), dot(-eyePos, b), dot(-eyePos, n));
    eyeDir = normalize(eyeDir);

    gl_Position = modelViewProjection * pos;
    distSquared = dot(diff, diff);
    shadowCoord = modelLightMatrix * pos;
    uvCoord = uvVertex;
    posModel = pos;
}