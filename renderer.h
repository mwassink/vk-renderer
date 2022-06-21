// Renderer, finally

#ifndef RENDERER_H
#define RENDERER_H


struct Buffer {
	VkBuffer handle;
	VkDeviceMemory memory;
	uint32_t size;
    VulkanContext* ctx;
	Buffer();
};


struct Texture {
    u32 w;
    u32 h;
    u32 size;
    u32 comps;
    VkImage imageHandle;
    VkImageView view;
    VkDeviceMemory mem;
    VkSampler sampler; // (TODO) allow for more of these
};


struct BasicRenderData {
    

    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout dsLayout;
    VkPipelineLayout plLayout;
    VkPipeline pipeline;
    VkRenderPass rPass;
    VkFramebuffer fb;
    VkExtent2D extent;

    
};

struct BasicVertexData {
    f32 x; f32 y;  f32 z; f32 w;
    f32 u; f32 v;
    f32 nx; f32 ny; f32 nz;
};

struct BasicDrawData {
    Buffer matrixUniforms;
    Buffer lightingData;
};

struct BasicModel {
    Vector<BasicVertexData> vData;
    Vector<u32> indices;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    Texture modelTexture;
    Quaternion rotation;
    f32 scale;
    u32 numPrimitives;
};






struct Renderer {
    
    VulkanContext ctx;

    Buffer MakeBuffer(u32 sizeIn, u32 flagsIn,  VkMemoryPropertyFlagBits wantedProperty);
    
    Buffer UniformBuffer( u32 sz, void* data);
    Buffer StagingBuffer( u32 sz, void* data);
    Buffer VertexBuffer( u32 sz, void* data);
    void toGPU(void* from, VkDeviceMemory mem, u32 sz );
    Texture RGBATexture(const char* fileName);
    void FillTexture(u32 sz, void* data, Texture* tex);
    void AllocMemoryImage(u32 sz, VkImage handle, VkMemoryPropertyFlagBits wantedProperty, VkDeviceMemory* mem);

    void BasicRenderPass(void);

    VkDescriptorSetLayout BasicDescriptorSetLayout(void);
    VkDescriptorPool BasicDescriptorPool(void);
    VkDescriptorSet BasicDescriptorSetAllocation(VkDescriptorPool* pool, VkDescriptorSetLayout* layout );
    void WriteBasicDescriptorSet(BasicDrawData* renderData, VkDescriptorSet* descriptorSet, BasicModel* model);
    
    void BasicPipelineLayout(BasicRenderData* renderData);
    VkRenderPass BasicRenderPass(VkFormat* swapChainFormat);
    VkPipeline BasicPipeline(BasicRenderData* renderData);
    VkShaderModule ShaderModule(const char* spirvFileName);
    void MoveUniformFromDMARegion(Buffer &stagingBuffer, Buffer &targetBuffer);
    void MoveVertexBufferFromDMARegion(Buffer &StagingBuffer, Buffer &targetBuffer);
    void BasicRenderModel(BasicModel* model);
    void DrawBasic(BasicRenderData* renderData, VkImageView* imgView, VkFramebuffer* currentFB, VkCommandBuffer cb, VkImage img, BasicModel* model);
    void RefreshFramebuffer(BasicRenderData* rData, VkImageView* imgView, VkFramebuffer* fb );
    void InitBasicRender(void);
    void AddBasicModel(BasicModel* model);
    void UpdateModel(BasicModel* model);
    void LightPass(void);
    void MoveBufferGeneric(Buffer& fromStaging, Buffer& to);
    Buffer IndexBuffer(u32 sz, void* data);
    

    

    
    
};

#endif
