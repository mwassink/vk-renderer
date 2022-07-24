#ifndef RASTERIZATION_RENDERER_H
#define RASTERIZATION_RENDERER_H

#include "basicrenderer.h"

struct Vertex{
    union {
        struct {
            f32 x; f32 y;  f32 z; f32 w;
            f32 u; f32 v;
            f32 nx; f32 ny; f32 nz;
            f32 tx; f32 ty; f32 tz;
        };
        struct {
            Vector4 coord;
            Vector2 uv;
            Vector3 normal;
            Vector3 tangent;
        };
    };
};

struct Model {
	Texture roughnessMap;
	Texture standardTexture;
	Texture normalMap;
	Texture environmentMap;
    Vector<Vertex> vertices;
    BasicMatrices matrices;
    VkDescriptorSet descriptorSet;
    Quaternion rotation;
    f32 scale;
    u32 numPrimitives;
    u32 matrixBufferOffset;
};



struct RasterizationRenderer : public BasicRenderer {
    VkDescriptorPool descriptorPool;
    VkPipeline shadowPipeline;
    VkPipeline scenePipeline;
    VkPipelineLayout sceneLayout;
    VkPipelineLayout shadowLayout;
    VkRenderPass sceneRenderPass;
    VkRenderPass shadowRenderPass;
    VkDescriptorPool DescriptorPool(u32 nDescriptors);
    

	
};


#endif
