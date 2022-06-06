// Renderer, finally

#ifndef RENDERER_H
#define RENDERER_H


struct Buffer {
	VkBuffer handle;
	VkDeviceMemory memory;
	uint32_t size;
    VulkanContext* ctx;
	Buffer();
    Buffer(VkDevice* dev, u32 size, u32 flags,  VkMemoryPropertyFlagBits memoryProperty);
    
    

};

struct UniformBuffer {
    Buffer internalBuffer;
    UniformBuffer(VkDevice* dev, u32 sz, void* data);
};

#if 0

struct VertexBuffer {
    Buffer internalBuffer;
    VertexBuffer(VkDevice* dev, u32 sz, void* data);
};
#endif





struct Renderer {
    
    VulkanContext ctx;
    
    void toGPU(void* from, VkDeviceMemory mem, u32 sz );

    
    
    
    
};

#endif
