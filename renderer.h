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
    Texture modelTexture;
    Buffer matrixUniforms;
    Buffer lightingData;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout dsLayout;
    VkPipelineLayout plLayout;
    VkRenderPass rPass;
};

struct BasicVertexData {
    f32 x; f32 y;  f32 z; f32 w;
    f32 u; f32 v;
    f32 nx; f32 ny; f32 nz;
};

struct BasicModel {
    Vector<BasicVertexData> vData;
    
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
    void WriteBasicDescriptorSet(BasicRenderData* renderData);
    
    VkPipelineLayout BasicPipelineLayout(BasicRenderData* renderData);
    VkRenderPass BasicRenderPass(VkFormat* swapChainFormat);
    VkPipeline BasicPipeline(BasicRenderData* renderData);
    VkShaderModule ShaderModule(const char* spirvFileName);
    void MoveUniformFromDMARegion(Buffer &stagingBuffer, Buffer &targetBuffer);
    void MoveVertexBufferFromDMARegion(Buffer &StagingBuffer, Buffer &targetBuffer);
    void BasicRenderModel(BasicModel* model);
    

    

    
    
};

#endif
