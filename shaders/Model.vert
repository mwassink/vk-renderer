#version 450

#define MAX_LIGHTS 100
#define MAX_OBJECTS 500

struct Matrices {
    mat4 modelViewProjection;
    mat4 modelView;
    mat4 model;
    mat3 normalMatrix;
};

struct LightTransforms {
    mat4 modelLightMatrix;
    vec3 lightCameraSpace;
};



layout(set=0, binding=0) uniform objectUniforms {
    Matrices matrices;
};

layout(set=0, binding=1) uniform lightUniforms {
    LightTransforms lightTransforms[100];
};


layout(location=0) in vec4 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 tangent;
layout(location=3) in vec2 uvVertex;
layout(location=4) in float handedness;


layout(location=0) out vec4 posWorldSpace;
layout(location=1) out vec2 uv;
layout(location=2) out vec3 n;
layout(location=3) out vec3 t;
layout(location=4) out vec4 b;





out gl_PerVertex {
    vec4 gl_Position;
}

void main(void) {
    n = normalize(normalMatrix * normal);
    t = normalize(normalMatrix * tangent);
    b = cross(n, t);
    b *= handedness;
    posWorldSpace = model * pos;
    uv = uvVertex;
    
}