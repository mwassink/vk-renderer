// 05/29/2022 22:17



#include "renderer.h"


#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#define MAXU32 0xFFFFFFFF


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
                if (vkBindBufferMemory(ctx.dev, buff.handle, buff.memory, 0) == VK_SUCCESS)
                    return buff;
            }
        }
    }

    ctx.pform.FatalError("Unable to alloc memory for a buffer", "Vulkan Runtime Error");
    return buff;
}

void Renderer::MoveBufferGeneric(Buffer& stagingBuffer, Buffer& targetBuffer ) {
    auto cmdBuffer = ctx.frameResources[0].commandBuffer;
    VkCommandBufferBeginInfo bInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0};
    vkBeginCommandBuffer(cmdBuffer, &bInfo);
    VkBufferCopy bcInfo = {0, 0, targetBuffer.size};
    vkCmdCopyBuffer(cmdBuffer, stagingBuffer.handle, targetBuffer.handle,1, &bcInfo);
    vkEndCommandBuffer(cmdBuffer);
    VkSubmitInfo sInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO, 0, 0, 0, 0, 1, &cmdBuffer, 0, 0};
    if (vkQueueSubmit (ctx.graphicsPresentQueue, 1, &sInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        ctx.pform.FatalError("Cannot submit queue", "Vulkan Runtime Error");
    }
    vkDeviceWaitIdle(ctx.dev);
    
}
void Renderer::MoveUniformFromDMARegion(Buffer &stagingBuffer, Buffer &targetBuffer) {
    auto cmdBuffer = ctx.frameResources[0].commandBuffer;
    
    VkCommandBufferBeginInfo beginInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        
      nullptr,                                            
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        
      nullptr                                             
    };

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy buffCopyInfo = {0, 0, targetBuffer.size};
    vkCmdCopyBuffer(cmdBuffer, stagingBuffer.handle, targetBuffer.handle, 1, &buffCopyInfo);
    VkBufferMemoryBarrier rwBarrier {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, targetBuffer.handle, 0, VK_WHOLE_SIZE
    };
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1, &rwBarrier, 0, nullptr);
    vkEndCommandBuffer(cmdBuffer);
    VkSubmitInfo subInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cmdBuffer, 0, nullptr
    };
    if (vkQueueSubmit(ctx.graphicsPresentQueue, 1, &subInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to submit to queue for making uniform buffer", "Vulkan Runtime Error");
    }
    vkDeviceWaitIdle(ctx.dev);
}

Buffer Renderer::StagingBuffer(u32 sz, void* data) {

    if (sz < stagingBuffer.size) {
        toGPU(data, stagingBuffer.memory, sz);
        return stagingBuffer;
    }
    // (TODO) cache an internal buffer if the size is fine
    Buffer internalBuffer = MakeBuffer(sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    toGPU(data, internalBuffer.memory, sz);

    return internalBuffer;
}

Buffer Renderer::VertexBuffer( u32 sz, void* data) {
    Buffer vBuffer = MakeBuffer(sz, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Buffer stagingBuffer = StagingBuffer(sz, data);

    MoveVertexBufferFromDMARegion(stagingBuffer, vBuffer);
    
    return vBuffer;

    
    

}

Buffer Renderer::IndexBuffer(u32 sz, void* data) {
    Buffer iBuffer= MakeBuffer(sz, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    Buffer sBuffer=  StagingBuffer(sz, data);
    MoveBufferGeneric(sBuffer, iBuffer);
    return iBuffer;
    
}

void Renderer::MoveVertexBufferFromDMARegion(Buffer &stagingBuffer, Buffer &targetBuffer) {
    auto cmdBuffer = ctx.frameResources[0].commandBuffer;
    
    VkCommandBufferBeginInfo beginInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        
      nullptr,                                            
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        
      nullptr                                             
    };

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy buffCopyInfo = {0, 0, targetBuffer.size};
    vkCmdCopyBuffer(cmdBuffer, stagingBuffer.handle, targetBuffer.handle, 1, &buffCopyInfo);
    VkBufferMemoryBarrier rwBarrier {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, targetBuffer.handle, 0, VK_WHOLE_SIZE
    };
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &rwBarrier, 0, nullptr);
    vkEndCommandBuffer(cmdBuffer);
    VkSubmitInfo subInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cmdBuffer, 0, nullptr
    };
    if (vkQueueSubmit(ctx.graphicsPresentQueue, 1, &subInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to submit to queue for making uniform buffer", "Vulkan Runtime Error");
    }
    vkDeviceWaitIdle(ctx.dev);

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

    FillTexture(imageData, &tex);
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

void Renderer::FillTexture(void* data, Texture* tex) {
    u32 sz = tex->comps * tex->w * tex->h;
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

VkDescriptorSetLayout Renderer::BasicDescriptorSetLayout() {
    VkDescriptorSetLayout basicLayout;

    Vector<VkDescriptorSetLayoutBinding> bindings(3);
    bindings[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT};
    bindings[1] = {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT};
    bindings[2] = {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT};
    VkDescriptorSetLayoutCreateInfo setCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, (u32)bindings.sz, bindings.data

    };
    if (vkCreateDescriptorSetLayout(ctx.dev, &setCreateInfo, nullptr, &basicLayout) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create basic descriptor set layout", "Vulkan Runtime Error");
    }

    return basicLayout;

}

VkDescriptorPool Renderer::BasicDescriptorPool(u32 nDescriptors) {
    VkDescriptorPool pool;
    Vector<VkDescriptorPoolSize> szes(3);
    szes[0] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors};
    szes[1] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors};
    szes[2] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors};

    VkDescriptorPoolCreateInfo cInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, nDescriptors, (u32) szes.sz, szes.data};
    if (vkCreateDescriptorPool(ctx.dev, &cInfo,nullptr, &pool  ) != VK_SUCCESS) {
        ctx.pform.FatalError("Could not create descriptor pool for basic shader", "Vulkan Runtime Error");
    }
    return pool;

}

VkDescriptorSet Renderer::BasicDescriptorSetAllocation(VkDescriptorPool* pool, VkDescriptorSetLayout* layout ) {

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, *pool, 1, layout
    };
    if (vkAllocateDescriptorSets(ctx.dev, &allocInfo,&descriptorSet) != VK_SUCCESS) {
        ctx.pform.FatalError("Could not make a descriptor set", "Vulkan Runtime Error");
    }

    return descriptorSet;
}

// (NOTE) This is where we put all of the data into the pipeline
void Renderer::WriteBasicDescriptorSet(VkDescriptorSet& descriptorSet, BasicModel* model) {
    VkDescriptorImageInfo imgInfo = {
        model->modelTexture.sampler, model->modelTexture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    VkDescriptorBufferInfo matrixUniforms = {
        uniformMegaPool.handle, model->matrixBufferOffset, sizeof(BasicMatrices)
    };
    VkDescriptorBufferInfo lightUniforms = {
        basicLightBuffer.handle, 0, sizeof(BasicLightData)
    };
    VkWriteDescriptorSet writes[3];

    writes[0] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
        nullptr, &matrixUniforms, nullptr
    };
    writes[1] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 1, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
        nullptr, &lightUniforms, nullptr
    };
    writes[2] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 2, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
        &imgInfo,nullptr,  nullptr
    };

    vkUpdateDescriptorSets(ctx.dev, (u32) 3, writes, 0, nullptr);

}

VkRenderPass Renderer::BasicRenderPass(VkFormat* format) {
    VkRenderPass rp;
    VkAttachmentDescription attachmentDescrs [] = {{
        0, *format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,  VK_ATTACHMENT_LOAD_OP_DONT_CARE,            
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
    };
    VkAttachmentReference colorReferences [] = { {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
    VkSubpassDescription subpassDescriptions[] = { {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, colorReferences, nullptr, nullptr, 0, nullptr}};
    VkRenderPassCreateInfo rpCreateInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0 ,1, attachmentDescrs, 1, subpassDescriptions, 0, nullptr
    };
    if (vkCreateRenderPass(ctx.dev, &rpCreateInfo, nullptr, &rp) != VK_SUCCESS) {
        ctx.pform.FatalError("Could not create render pass", "Vulkan Runtime Error");
    }

    return rp;
}


void Renderer::BasicPipelineLayout(BasicRenderData* renderData) {
    VkPipelineLayout plLayout;    
    VkPipelineLayoutCreateInfo layoutCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 1, &renderData->dsLayout, 0, nullptr
    };
    if (vkCreatePipelineLayout(ctx.dev, &layoutCreateInfo, nullptr, &plLayout) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create a basic pipeline layout", "Vulkan Runtime Error");
    }
    renderData->plLayout = plLayout;
}


VkShaderModule Renderer::ShaderModule(const char* spirvFileName) {
    VkShaderModule mod;
    char errBuff[1000] = {0};
    FileData data = ctx.pform.ReadBinaryFile(spirvFileName);
    VkShaderModuleCreateInfo crInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, data.size, (u32*) data.data};
    if (vkCreateShaderModule(ctx.dev, &crInfo, nullptr, &mod) != VK_SUCCESS) {
        snprintf(errBuff, 1000, "Unable to load shader %s", spirvFileName);
        ctx.pform.FatalError(errBuff, "Vulkan Runtime Error");
    }
    return mod;
}


void Renderer::BasicPipeline(BasicRenderData* renderData) {
    VkPipeline pl;

    auto vMod = ShaderModule("shaders/basicVertex.spv");
    auto pMod = ShaderModule("shaders/basicPixel.spv");
    VkPipelineShaderStageCreateInfo shaders[] = {
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vMod, "main", nullptr},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, pMod, "main", nullptr}
    };
    
    VkVertexInputBindingDescription bindingDescription = {
        0, sizeof(BasicVertexData)
    };

    VkVertexInputAttributeDescription vertexAttrDescrs []  = {
        {0, bindingDescription.binding, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        {1, bindingDescription.binding, VK_FORMAT_R32G32_SFLOAT, 4 * sizeof(f32)},  // this is the offset
        {2, bindingDescription.binding, VK_FORMAT_R32G32B32A32_SFLOAT}
    };

    VkPipelineVertexInputStateCreateInfo inStateCInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1 ,&bindingDescription,3, vertexAttrDescrs 
    };
    VkPipelineInputAssemblyStateCreateInfo inAsmCrInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,VK_FALSE 
    };
    VkPipelineViewportStateCreateInfo vpStateCrInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 1, nullptr, 1, nullptr
    };
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,nullptr, 0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f 
    };

    VkPipelineMultisampleStateCreateInfo multiStateCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0, VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE
    };

    VkPipelineColorBlendAttachmentState cBlendState = {
        VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT |VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT  
    };

    VkPipelineColorBlendStateCreateInfo cbStateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 1, &cBlendState, {0, 0, 0, 0}
    };

    VkDynamicState dStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}; // we can change these
    VkPipelineDynamicStateCreateInfo dStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 2, dStates};
    VkGraphicsPipelineCreateInfo pCrInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0, 2, shaders, &inStateCInfo, &inAsmCrInfo, nullptr,
        &vpStateCrInfo, &rasterizationStateCreateInfo, &multiStateCreateInfo, nullptr, &cbStateInfo, &dStateCreateInfo, renderData->plLayout,
        renderData->rPass, 0, VK_NULL_HANDLE, -1
    };
    if (vkCreateGraphicsPipelines(ctx.dev, VK_NULL_HANDLE, 1, &pCrInfo, nullptr, &pl ) != VK_SUCCESS) {
        ctx.pform.FatalError("Could not create the basic pipeline" ,"Vulkan Runtime Error");
    }

    renderData->pipeline = pl;
}

void Renderer::RefreshFramebuffer(BasicRenderData* rData, VkImageView* imgView, VkFramebuffer* currFB) {
    if (*currFB != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(ctx.dev, *currFB, nullptr);
        *currFB = VK_NULL_HANDLE;
    }

    VkFramebufferCreateInfo fbCrInfo = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr,
        0, rData->rPass, 1, imgView,
        ctx.ext.width, ctx.ext.height, 1
    };

    if (vkCreateFramebuffer(ctx.dev, &fbCrInfo, nullptr, currFB) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create a framebuffer!", "Vulkan Runtime Error");
    }
    
}

// Initialize the data before calling this

// Make the pipeline, render passes, etc
void Renderer::InitBasicRender(u32 numDescriptors) {
    
    rData.dsLayout = BasicDescriptorSetLayout();
    basicLayout = rData.dsLayout;
    BasicPipelineLayout(&rData);
    auto rp = BasicRenderPass(&ctx.sc.format.format);
    rData.rPass = rp;
    auto pool = BasicDescriptorPool(numDescriptors);
    descriptorPool = pool;
    auto set = BasicDescriptorSetAllocation(&pool, &rData.dsLayout);
    rData.descriptorSet = set;
    BasicPipeline(&rData);
    
}

// Make a vertex buffer, get textures, etc
// The model is supposed to have all vertex data already in the struct, except for the file
BasicModel Renderer::AddBasicModel(BasicModelFiles fileNames) {
    
    BasicModel model = LoadModelObj(fileNames.objFile, fileNames.rgbaName);
    model.vertexBuffer = VertexBuffer(model.vData.sz * sizeof (BasicVertexData), model.vData.data);
    model.indexBuffer = IndexBuffer(model.indices.sz * sizeof(u32), model.indices.data);
    model.descriptorSet = BasicDescriptorSetAllocation(&descriptorPool, &basicLayout);

    return model;
}

Light Renderer::AddLight(Vector4 pos, Vector4 color, f32 power) {
    Light l;
    BasicLightData& ld = l.lightData;
    ld.lightColor = color;
    ld.position = pos;
    ld.power = power;

    return l;

}


Light Renderer::AddLight(const BasicLightData* lightData) {
    Light l;
    l.lightData = *lightData;
    return l;
}



BasicModel Renderer::LoadModelObj(const char* f, const char* imageFile) {
    BasicModel model;

    Vector2 dummyUV;
    Vector3 dummyVec;
    Vector<Vector3> coords, normals;
    Vector<Vector2> tcoords;
    Vector<BasicVertexData> verticesList;
    Vector<u32> indices;
    coords.push(dummyVec);
    normals.push(dummyVec);
    tcoords.push(dummyUV);
    
    
    // is an estimate
    int triangles = CountTrianglesOccurences(f);
    if (triangles == -1) {
        pform->FatalError("Invalid asset file","Vulkan Runtime Error"); 
    }
    HashTable mappingTable(triangles * 4);
    
    FILE* fp = fopen(f, "rb");
    
    if (!fp) {
        pform->FatalError("unable to read in obj file","Vulkan Runtime Error"); 
    }
    char arr[200];
    char type[100];
    char tri1[80]; char tri2[80]; char tri3[80]; char tri4[80];
    Vector3 tr;
    Vector2 df;
    
    while (fgets(arr, 100, fp) != NULL) {
        sscanf(arr, "%s", type );
        if (!strcmp(type, "v")) {
            sscanf(arr, "%s %f %f %f", type, &tr.x, &tr.y, &tr.z);
            coords.push(tr);
        }
        else if (!strcmp(type, "vt")) {
            sscanf(arr, "%s %f %f", type, &df.u, &df.v);
            tcoords.push(df);
        }
        else if (!strcmp(type, "vn")) {
            sscanf(arr, "%s %f %f %f", type, &tr.x, &tr.y, &tr.z);
            normals.push(tr);
        }
        else if (!strcmp(type, "f")) {
            if(sscanf(arr, "%s %s %s %s %s", type, tri1, tri2, tri3, tri4) == 5) {
                // 1, 2, 3 gives a properly wound triangle
                ParseVertex(tri1, &mappingTable, &indices, &coords, &normals, &tcoords, &verticesList );
                ParseVertex(tri2, &mappingTable, &indices, &coords, &normals, &tcoords, &verticesList);
                ParseVertex(tri3, &mappingTable, &indices, &coords, &normals, &tcoords, &verticesList);
                // 1, 3, 4 should?
                ParseVertex(tri1, &mappingTable, &indices, &coords, &normals, &tcoords, &verticesList );
                ParseVertex(tri3, &mappingTable, &indices, &coords, &normals, &tcoords, &verticesList);
                ParseVertex(tri4, &mappingTable, &indices, &coords, &normals, &tcoords, &verticesList);
            }
            
            ParseVertex(tri1, &mappingTable, &indices, &coords, &normals, &tcoords, &verticesList );
            ParseVertex(tri2, &mappingTable, &indices, &coords, &normals, &tcoords, &verticesList);
            ParseVertex(tri3, &mappingTable, &indices, &coords, &normals, &tcoords, &verticesList);
            
        }
    }

    fclose(fp);
    model.indices = indices;
    model.vData = verticesList;
    model.modelTexture = RGBATexture(imageFile);
    model.numPrimitives = indices.sz;
    
    
    return model;
}


u32 Renderer::CountTrianglesOccurences(const char* f) {
    int sz, ctr = 0;
    auto data = pform->ReadBinaryFile(f);
    
    if (!data.data) return -1;
    for (int i = 0; i < data.size; ++i) {
        ctr += (data.data[i] == 'f');
    }
    pform->ReleaseFileData(&data);
    return ctr;   
}

BasicVertexData Renderer::ConstructVertex(Vector<Vector3>* coords, Vector<Vector3>* normals, Vector<Vector2>* uvcoords, u32 p, u32 t, u32 n  ) {
    BasicVertexData v;
    v.coord = Vector4((*coords)[p], 1.0f);
    if (n != MAXU32)
        v.normal = (*normals)[n];
    else
        v.normal = {0.0f, 0.0f, 0.0f};
    if (n != MAXU32)
        v.uv = (*uvcoords)[t];
    else
        v.uv = {2.5f, 2.5f};
    return v;
    
}


void Renderer::ParseVertex(const char* s, HashTable* indexHashTable, Vector<u32>* indices, Vector<Vector3>* coords, Vector<Vector3>* normals, Vector<Vector2>* uvcoords,
                 Vector<BasicVertexData>* vertices) {
    unsigned int p, t, n, occurences;
    occurences = CountOccurrences(s, '/', 0);
    
    if (occurences == 2) {
        if (sscanf(s, "%u/%u/%u", &p, &t, &n ) != 3) {
            t = MAXU32;
            sscanf(s, "%u//%u", &p, &n );
        }
    }
    else if (occurences == 1) {
        n = MAXU32;
        sscanf(s, "%u/%u", &p, &t);
    }
    else {
        t = MAXU32; n = MAXU32;
        sscanf(s, "%u", &p);
    }
    // Put the res in the hash table
    u32 val = indexHashTable->at(p, t, n, 0);
    if (val == -1) {
        val = indexHashTable->insert(p, t, n, 0);
        vertices->push(ConstructVertex(coords, normals, uvcoords, p, t, n));
    }
    
    indices->push(val);  
}

void Renderer::Init(void){
    ctx.Init();
    
}

void Renderer::DrawBasicFlatScene(BasicFlatScene* scene) {

    Vector<BasicModel>& models = scene->models;
    Vector<Light>& lights = scene->lights;
    
    PerFrameData* pfd = &ctx.frameResources[ctx.currFrame];

    if (vkWaitForFences(ctx.dev, 1, &pfd->fence, VK_FALSE, 1000000000) != VK_SUCCESS) {
      ctx.pform.FatalError("Error while waiting on fence", "VK Runtime Error");
    }

    vkResetFences(ctx.dev, 1, &pfd->fence);
    u32 imgIndex;
    
    auto res = vkAcquireNextImageKHR(ctx.dev, ctx.sc.handle, UINT64_MAX,
                                     pfd->presentReady, VK_NULL_HANDLE, &imgIndex);

    switch (res) {
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR:
      break;
    case VK_ERROR_OUT_OF_DATE_KHR:
      ctx.WindowChange();
      return;
    default:
      ctx.pform.FatalError("Swapchain error during acquisition", "VK Runtime Error");
    }

    
    SetupUniforms(scene);
    VkCommandBuffer& cb = pfd->commandBuffer;
    // hopefully this happens only once?

    Image& img = ctx.images[ctx.currFrame];
    RefreshFramebuffer(&rData, &img.view, &pfd->framebuffer );

    // write the descriptors with the first light
    Light dummyLight = {0};
    UpdateLightUniform(&dummyLight, cb, false);
    VkCommandBufferBeginInfo cbInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 0,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0
    };

    for (int i = 0; i < models.sz; i++) {
        WriteBasicDescriptorSet(models[i].descriptorSet, &models[i]);
    }
        
    vkBeginCommandBuffer(pfd->commandBuffer, &cbInfo );
    if (uniformPoolNeedsCopy) {
        CopyUniformPool(pfd->commandBuffer);
        uniformPoolNeedsCopy = false;
    }


    

        VkImageSubresourceRange srr = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,0,1};
    VkImageMemoryBarrier prDrawBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, 0, VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        ctx.gqFamilyIndex, ctx.gqFamilyIndex, img.handle, srr
    };
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &prDrawBarrier);
    
    VkClearValue clearVal = {1.0f, 1.0f, 1.0f, 1.0f};
    VkRect2D rect = { {0, 0}, ctx.ext};
    
        
    VkRenderPassBeginInfo rpBeginInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, 0, rData.rPass,
        pfd->framebuffer, rect, 1, &clearVal
    };

    

    
    for (int i = 0; i < lights.sz; i++ ) {
        auto light = lights[i];

        UpdateLightUniform(&light, cb, true);
        vkCmdBeginRenderPass(cb, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,  rData.pipeline);
        VkViewport viewport = { 0, 0, static_cast<float>(ctx.ext.width),static_cast<float>(ctx.ext.height), 0, 1 };
        VkRect2D scissor = { {0, 0}, {static_cast<u32>(ctx.ext.width), static_cast<u32>(ctx.ext.height)}};
        VkDeviceSize offset = 0;
    
        vkCmdSetViewport( cb, 0, 1, &viewport );
        vkCmdSetScissor( cb, 0, 1, &scissor );

        for (int j = 0; j < models.sz; j++) {

            BasicModel& model = models[j];
            
            vkCmdBindVertexBuffers(cb, 0, 1, &model.vertexBuffer.handle, &offset);
            vkCmdBindIndexBuffer(cb, model.indexBuffer.handle, offset, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(cb ,VK_PIPELINE_BIND_POINT_GRAPHICS, rData.plLayout, 0, 1, &model.descriptorSet, 0, 0);
            vkCmdDrawIndexed(cb, model.numPrimitives, 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(cb);
    }    

    VkImageMemoryBarrier drawPresBarrier = {
              VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,  
              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, ctx.gqFamilyIndex, ctx.gqFamilyIndex,
              img.handle, srr
    };
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &drawPresBarrier );
    if (vkEndCommandBuffer(cb) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to record basic draw command buffer", "Vulkan Runtime Error" );
    }

    VkPipelineStageFlags v = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo sInfo = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1,
      &pfd->presentReady, &v,
      1, &pfd->commandBuffer,1, &pfd->renderingReady
    };

    if (vkQueueSubmit(ctx.graphicsPresentQueue, 1, &sInfo, pfd->fence) != VK_SUCCESS) {
      return;
    }
    VkPresentInfoKHR pInfo = {
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, 1,
      &pfd->renderingReady, 1, &ctx.sc.handle,
      &imgIndex, nullptr
    };

    res = vkQueuePresentKHR(ctx.graphicsPresentQueue, &pInfo);

    
    switch (res) {
    case VK_SUCCESS:
      break;
    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
      ctx.WindowChange();
      return;
    default:
      ctx.pform.FatalError("Swapchain error during acquisition", "VK Runtime Error");
    }

    

    ctx.currFrame = (ctx.currFrame + 1) % NUM_IMAGES;
}

BasicFlatScene Renderer::SimpleScene(BasicModel* modelsIn, u32 numModels, BasicLightData* lightsIn, u32 numLights) {
    BasicFlatScene s;

    Vector<BasicModel> models(numModels);
    Vector<Light> lights(numLights);

    for (u32 i = 0;  i < numModels; i++) {
        models[i] = modelsIn[i];
    }
    for (u32 i = 0; i < numLights; i++) {
        lights[i] = AddLight(&lightsIn[i]);
    }
    models.sz = numModels;
    lights.sz = numLights;
    s.models = models;
    s.lights = lights;
    return s;
}

void Renderer::SetupUniforms(BasicFlatScene * scene ) {
  // 7/4/2022
  //Make a staging buffer for all of these
  // Move the stuff from the staging buffer and assign each of these
  // an offset

  u32 neededSpace = sizeof(BasicMatrices) * scene->models.sz;

  uniformHostPool = MakeBuffer(neededSpace, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  
  
  
  void* loc;
  if (vkMapMemory(ctx.dev, uniformHostPool.memory, 0, neededSpace, 0, &loc) != VK_SUCCESS) {
    ctx.pform.FatalError("Unable to map memory for staging buffer", "VK Runtime Error");
    
  }
  u32 spaceCounter = 0;
  char* copyTarget = (char*) loc;
  for (int i = 0; i < scene->models.sz; i++) {
    scene->models[i].matrixBufferOffset = spaceCounter;
    memcpy(copyTarget, &scene->models[i].matrices, sizeof(BasicMatrices));
    copyTarget += sizeof(BasicMatrices);
    spaceCounter += sizeof(BasicMatrices);
  }
#if 0
  
  for (int i = 0; i < scene->lights.sz; i++) {
    scene->lights[i].lightBufferOffset = spaceCounter;
    memcpy(copyTarget, &scene->lights[i].lightData, sizeof(BasicLightData));
    copyTarget += sizeof(BasicLightData);
    spaceCounter += sizeof(BasicLightData);
  }
#endif
  VkMappedMemoryRange flush = {
    VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr,
        uniformHostPool.memory, 0, neededSpace
  };
  
  vkFlushMappedMemoryRanges(ctx.dev, 1,&flush );
  vkUnmapMemory(ctx.dev, uniformHostPool.memory);

  uniformMegaPool = MakeBuffer(neededSpace, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  uniformPoolNeedsCopy = true;
     
}

// 07/04/2022 REQUIRES command buffers to be in the recording state
void Renderer::CopyUniformPool(VkCommandBuffer& cb) {

    VkBufferCopy buffCopyInfo = {0, 0, uniformMegaPool.size };
    vkCmdCopyBuffer(cb, uniformHostPool.handle, uniformMegaPool.handle, 1, &buffCopyInfo);
    VkBufferMemoryBarrier rwBarrier {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, uniformMegaPool.handle, 0, VK_WHOLE_SIZE
    };
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1, &rwBarrier, 0, nullptr);
    
    
}


void Renderer::UpdateLightUniform(Light* light, VkCommandBuffer& cb, bool recording = true) {

    Buffer tmpBuffer = StagingBuffer(sizeof(BasicLightData), light);
    if (!lbOK) {
        basicLightBuffer = MakeBuffer(sizeof(BasicLightData), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );        
    }
    if (!recording) return;
    VkBufferCopy buffCopyInfo = {0, 0, tmpBuffer.size };
    vkCmdCopyBuffer(cb, tmpBuffer.handle, basicLightBuffer.handle, 1, &buffCopyInfo);
    VkBufferMemoryBarrier rwBarrier {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, uniformMegaPool.handle, 0, VK_WHOLE_SIZE
    };
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1, &rwBarrier, 0, nullptr);
}
