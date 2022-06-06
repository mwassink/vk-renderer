// 05/29/2022 22:17


#include "types.h"
#include "device.h"
#include "renderer.h"

#include "functions.h"




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

UniformBuffer::UniformBuffer(VkDevice* dev, u32 sz, void* data) {
    internalBuffer = Buffer(dev, sz, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    // (TODO) complete this!

    
    
    
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


