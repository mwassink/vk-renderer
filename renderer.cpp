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

Buffer Renderer::MakeBuffer(u32 sizeIn, u32 flagsIn,  VkMemoryPropertyFlagBits wantedProperty) {
    Buffer buff;
    buff.size = sizeIn;
    VkBufferCreateInfo bufferCreateInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        buff.size,
        flagsIn,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    if (vkCreateBuffer(ctx.dev, &bufferCreateInfo, nullptr, &buff.handle) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to allocate buffer entry", "Vulkan runtime Error");
    }

    VkMemoryRequirements bReqs;
    VkPhysicalDeviceMemoryProperties  physProps;
    vkGetBufferMemoryRequirements(ctx.dev, buff.handle, &bReqs);
    vkGetPhysicalDeviceMemoryProperties(ctx.gpu, &physProps);

    for (u32 i = 0; i < physProps.memoryTypeCount; i++) {
        // probably a list of flags to see which types are supported. If it is supported and it matches the property then
        if ((bReqs.memoryTypeBits & (i << i)) && (physProps.memoryTypes[i].propertyFlags & wantedProperty)) {
            VkMemoryAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                nullptr,
                bReqs.size,
                i
            };
            if (vkAllocateMemory(ctx.dev, &allocInfo, nullptr, &buff.memory) == VK_SUCCESS) {
                return buff;
            }
        }
    }

    ctx.pform.FatalError("Unable to alloc memory for a buffer", "Vulkan Runtime Error");
    return buff;
}

Buffer Renderer::UniformBuffer(u32 sz, void* data) {
    
    Buffer uniformBuffer = MakeBuffer(sz, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    // CPU can't do DMA on local memory, so that's what the staging buffer is for

    Buffer stagingBuffer = StagingBuffer(sz, data);
    auto cmdBuffer = ctx.frameResources[0].commandBuffer;
    
    VkCommandBufferBeginInfo beginInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        
      nullptr,                                            
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        
      nullptr                                             
    };

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy buffCopyInfo = {0, 0, uniformBuffer.size};
    vkCmdCopyBuffer(cmdBuffer, stagingBuffer.handle, uniformBuffer.handle, 1, &buffCopyInfo);
    VkBufferMemoryBarrier rwBarrier {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, uniformBuffer.handle, 0, VK_WHOLE_SIZE
    };
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1, &rwBarrier, 0, nullptr);
    VkSubmitInfo subInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cmdBuffer, 0, nullptr
    };
    if (vkQueueSubmit(ctx.graphicsPresentQueue, 1, &subInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to submit to queue for making uniform buffer", "Vulkan Runtime Error");
    }


    vkDeviceWaitIdle(ctx.dev);
  
    
}

Buffer Renderer::StagingBuffer(u32 sz, void* data) {
    
    Buffer internalBuffer = MakeBuffer(sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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
    if (!imageData || !(w > 0 && h > 0 && comps == 4)) {
        ctx.pform.FatalError(fileName, "Error while reading texture");
    }

    tex.size = w * h * 4;
    tex.w = w;
    tex.h = h;
    tex.comps = comps;

    
    VkImageCreateInfo cInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                  
        nullptr,                                              
        0,                                                    
        VK_IMAGE_TYPE_2D,                                     
        VK_FORMAT_R8G8B8A8_UNORM,                             
        {                                                     
            tex.w,                                                
            tex.h,                                               
            1
        },
        1,                                                    
        1,                                                    
        VK_SAMPLE_COUNT_1_BIT,                                
        VK_IMAGE_TILING_OPTIMAL,                              
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |                     
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_SHARING_MODE_EXCLUSIVE,                            
        0,                                                    
        nullptr,                                              
        VK_IMAGE_LAYOUT_UNDEFINED   
    };

    if (vkCreateImage(ctx.dev, &cInfo,nullptr, &tex.imageHandle ) != VK_SUCCESS) {
        ctx.pform.FatalError("Vk Image Creation Error ", "VK Runtime Error");
    }

    return tex;
    
}


void Renderer::AllocMemoryImage(u32 sz, VkImage handle, VkMemoryPropertyFlagBits wantedProperty, VkDeviceMemory* mem) {
    VkMemoryRequirements iReqs;
    VkPhysicalDeviceMemoryProperties  physProps;
    vkGetImageMemoryRequirements(ctx.dev, handle, &iReqs);
    vkGetPhysicalDeviceMemoryProperties(ctx.gpu, &physProps);

    for (u32 i = 0; i < physProps.memoryTypeCount; i++) {
        // probably a list of flags to see which types are supported. If it is supported and it matches the property then
        if ((iReqs.memoryTypeBits & (i << i)) && (physProps.memoryTypes[i].propertyFlags & wantedProperty)) {
            VkMemoryAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                nullptr,
                iReqs.size,
                i
            };
            if (vkAllocateMemory(ctx.dev, &allocInfo, nullptr, mem) == VK_SUCCESS) {
                return ;
            }
        }
    }
    ctx.pform.FatalError("Error while allocating image memory", "Vulkan Runtime Error");
}

void Renderer::FillTexture(u32 sz, void* data, Texture* tex) {
    AllocMemoryImage(sz, tex->imageHandle, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &tex->mem);
    if ( vkBindImageMemory(ctx.dev, tex->imageHandle, tex->mem, 0) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to bind image to mem", "VK Runtime Error");
    }

    VkImageViewCreateInfo viewCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        tex->imageHandle, VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM, 
        {
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY
        },
        {
            VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
        }
    };
    vkCreateImageView(ctx.dev, &viewCreateInfo, nullptr, &tex->view);

    VkSamplerCreateInfo samplerCreateInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0 , VK_FILTER_LINEAR, VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        0.0f, VK_FALSE, 1.0f, VK_FALSE, VK_COMPARE_OP_ALWAYS, 0.0f, 0.0f, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, VK_FALSE 
    };

    vkCreateSampler(ctx.dev, &samplerCreateInfo, nullptr, &tex->sampler);
    // Copy into the buffer
    Buffer stagingBuffer = StagingBuffer(sz, data);
    // (TODO) destroy buffer

    // kick off command buffer to
    VkCommandBufferBeginInfo cmdBuffBeginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // cannot be recycled I guess
        nullptr
    };

    
    VkCommandBuffer cmdBuffer = ctx.frameResources[0].commandBuffer;
    vkBeginCommandBuffer(cmdBuffer, &cmdBuffBeginInfo);
    VkImageSubresourceRange imgSubRange  = {
        VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
    };

    VkImageMemoryBarrier imageBarrierToTransfer = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr, 0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        tex->imageHandle,
        imgSubRange
    };
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
        nullptr, 0, nullptr, 1, &imageBarrierToTransfer);


    VkBufferImageCopy copyInfo = {0, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {0, 0, 0}, {tex->w, tex->h, 1}};

    
    // We want optimal image layout
    vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer.handle, tex->imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

    VkImageMemoryBarrier preventShaderRead = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED, tex->imageHandle,
        imgSubRange 
    };
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &preventShaderRead);
    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cmdBuffer, 0, nullptr
    };

    if (vkQueueSubmit(ctx.graphicsPresentQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create a texture", "Vulkan Runtime Error");
    }

    vkDeviceWaitIdle(ctx.dev);
}

VkDescriptorSetLayout BasicDescriptorSetLayout() {
    VkDescriptorSetLayout basicLayout;

    Vector<VkDescriptorSetLayoutBinding> bindings(2);
    bindings[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, }


}







