// 05/29/2022 22:17


#include "types.h"
#include "device.h"
#include "renderer.h"

#include "functions.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"




Buffer::Buffer(){
		handle = VK_NULL_HANDLE;
		memory = VK_NULL_HANDLE;
		size = 0;
}

Buffer::Buffer(VkDevice* dev, u32 sizeIn, u32 flagsIn,  VkMemoryPropertyFlagBits wantedProperty) {
    size = sizeIn;
    VkBufferCreateInfo bufferCreateInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        size,
        flagsIn,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    if (vkCreateBuffer(ctx->dev, &bufferCreateInfo, nullptr, &handle) != VK_SUCCESS) {
        ctx->pform.FatalError("Unable to allocate buffer entry", "Vulkan runtime Error");
    }

    VkMemoryRequirements bReqs;
    VkPhysicalDeviceMemoryProperties  physProps;
    vkGetBufferMemoryRequirements(ctx->dev, handle, &bReqs);
    vkGetPhysicalDeviceMemoryProperties(ctx->gpu, &physProps);

    for (u32 i = 0; i < physProps.memoryTypeCount; i++) {
        // probably a list of flags to see which types are supported. If it is supported and it matches the property then
        if ((bReqs.memoryTypeBits & (i << i)) && (physProps.memoryTypes[i].propertyFlags & wantedProperty)) {
            VkMemoryAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                nullptr,
                bReqs.size,
                i
            };
            if (vkAllocateMemory(ctx->dev, &allocInfo, nullptr, &memory) == VK_SUCCESS) {
                return;
            }
        }
    }

    ctx->pform.FatalError("Unable to alloc memory for a buffer", "Vulkan Runtime Error");
    
}

Buffer Renderer::UniformBuffer(u32 sz, void* data) {
    
    Buffer internalBuffer = Buffer(&ctx.dev, sz, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    // Move the data into the buffer
    toGPU(data, internalBuffer.memory, sz);
    return internalBuffer;
}

Buffer Renderer::StagingBuffer(u32 sz, void* data) {
    
    Buffer internalBuffer = Buffer(&ctx.dev, sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    toGPU(data, internalBuffer.memory, sz);

    return internalBuffer;
}



void Renderer::toGPU(void* data, VkDeviceMemory mem, u32 sz ) {
    
    void* map_location;
    if (vkMapMemory(ctx.dev,  mem, 0, sz, 0, &map_location) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to map memory", "Vulkan Runtime Error");
    }
    memcpy(map_location, data, sz);
    VkMappedMemoryRange flushable = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr,
        mem, 0, sz
    };

    vkFlushMappedMemoryRanges(ctx.dev, 1, &flushable);

    vkUnmapMemory(ctx.dev, mem);
    
}

Texture Renderer::RGBATexture(const char* fileName) {
    Texture tex;
    FileData data = ctx.pform.ReadBinaryFile(fileName);
    int w, h, comps;
    u8* imageData = stbi_load_from_memory( (u8*) data.data, data.size, &w, &h, &comps, 4  );
    ctx.pform.ReleaseFileData(&data);
    if (!imageData || !(w > 0 && h > 0 && comps > 0)) {
        ctx.pform.FatalError(fileName, "Error while reading texture");
    }

    
    


    return tex;
    
}






