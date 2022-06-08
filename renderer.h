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


struct Texture {
    u32 w; u32 h;
    VkImage imageHandle;
    
};





struct Renderer {
    
    VulkanContext ctx;


    Buffer UniformBuffer( u32 sz, void* data);
    Buffer StagingBuffer( u32 sz, void* data);
    void toGPU(void* from, VkDeviceMemory mem, u32 sz );
    Texture RGBATexture(const char* fileName);
    
    
    
    
    
};

#endif
