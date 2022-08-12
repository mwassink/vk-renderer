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

struct FBAttachment {
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
    VkFormat format;

    FBAttachment(VkFormat format, VkImageUsageFlags flags, u32 w, u32 h, VulkanContext& ctx);
    FBAttachment() = default;

};

struct PushGatherFrag{
    int lnum;
    int objNum;
    Vector3 f0;
};

struct GBufferAttachments {
    union {
        struct {
            FBAttachment normals;
            FBAttachment diffuseColor;
            FBAttachment specularColor;
            FBAttachment f0Out;
            FBAttachment roughness;
            FBAttachment pos;
            FBAttachment depth;
        };
        struct {
            FBAttachment attachments[7];
        };
    };
};



struct RasterizationRenderer : public BasicRenderer {

    static VkFormat const DepthFormat = VK_FORMAT_D16_UNORM;

    static const int numColorAttachments = 6;
    static const int numDepthAttachments = 1;
    static const int totalAttachments = numColorAttachments + numDepthAttachments;


    VkDescriptorPool descriptorPool;    
    VkPipelineLayout sceneLayout;
    VkRenderPass renderPassGather;
    VkRenderPass renderPassDraw;
    VkSampler colorSampler;
    VkPipeline pipelineGather;
    VkPipeline pipelineDraw;
    GBufferAttachments gBufferAttachments;
    
    VkDescriptorSetLayout DescriptorSetLayoutGatherPass();
    VkDescriptorSetLayout DescriptorSetLayoutDraw();
    VkDescriptorPool DescriptorPoolGatherPass(u32 nDescriptors);
    VkDescriptorPool DescriptorPoolDraw(u32 nDescriptors);
    // the pipeline code is reused
    VkPipeline Pipeline(u32 mode, GBufferAttachments& attachments);
    VkPipelineLayout PipelineLayoutGatherPass(VkDescriptorSetLayout& dsLayout);
    VkPipelineLayout PipelineLayoutDraw(VkDescriptorSetLayout& dsLayout);
    void CreateAttachments(GBufferAttachments* attachments, u32 w, u32 h);
    VkRenderPass RenderPassGatherPass(GBufferAttachments& attachments);
    void Init();
    void WriteDescriptorSets(VkDescriptorSet& ds, Buffer* buffers, u32* sizes, Texture* textures);
    
};


#endif
