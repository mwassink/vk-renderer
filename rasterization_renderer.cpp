
#include "rasterization_renderer.h"

VkDescriptorPool RasterizationRenderer::DescriptorPoolGatherPass(u32 nDescriptors) {
    VkDescriptorPool pool;
    Vector<VkDescriptorPoolSize> szes(7);
    szes[0] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors }; //shadows
    szes[1] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors }; //matrix
    szes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors }; // light
    szes[3] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors}; //roughness
    szes[4] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors}; //diffuse
    szes[5] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors}; //normal
    szes[6] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors}; //specular
    

    VkDescriptorPoolCreateInfo cInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, nDescriptors, (u32)szes.sz, szes.data };
    if (vkCreateDescriptorPool(ctx.dev, &cInfo, nullptr, &pool) != VK_SUCCESS) {
        ctx.pform.FatalError("Could not create descriptor pool for basic shader", "Vulkan Runtime Error");
    }
    return pool;
}


VkPipeline RasterizationRenderer::PipelineGatherPass(u32 mode) {

    
    VkShaderModule vMod; 
    VkShaderModule pMod;
    VkPipelineShaderStageCreateInfo shaders[2];
    VkPipelineVertexInputStateCreateInfo inStateCInfo;
    if (mode == 0) {
        vMod = ShaderModule("shaders/Model.vert");
        pMod = ShaderModule("shaders/Model.frag");

        shaders[0] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vMod, "main", nullptr };
        shaders[1] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, pMod, "main", nullptr };


        VkVertexInputBindingDescription bindingDescription = {
            0, sizeof(Vertex)
        };
        u32 binding = bindingDescription.binding;
        VkVertexInputAttributeDescription vertexAttrDescrs[] = {
            {0, binding, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
            {1, binding, VK_FORMAT_R32G32_SFLOAT, 4 * sizeof(f32)},
            {2, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
            {3, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)}
        };

        VkPipelineVertexInputStateCreateInfo tmp = {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &bindingDescription, 4, vertexAttrDescrs
        };
        inStateCInfo = tmp;
    }
    else if (mode == 1) {
        vMod = ShaderModule("shaders/Shadow.vert");
        pMod = ShaderModule("shaders/Shadow.frag");


        VkPipelineShaderStageCreateInfo shaders[] = {
            {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vMod, "main", nullptr},
            {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, pMod, "main", nullptr}
        };

        VkVertexInputBindingDescription bindingDescription = {
            0, sizeof(Vertex)
        };
        auto binding = bindingDescription.binding;
        VkVertexInputAttributeDescription vertexAttrDescrs[] = {
            {0, binding, VK_FORMAT_R32G32B32A32_SFLOAT, 0}
        };

        VkPipelineVertexInputStateCreateInfo tmp = {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &bindingDescription, 1, vertexAttrDescrs
        };
        inStateCInfo = tmp;
    }
    

    VkPipeline pl;
    VkPipelineInputAssemblyStateCreateInfo inAsmCreateInfo = {
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

    VkDynamicState dState[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr,
        0, 2, dState};

    VkGraphicsPipelineCreateInfo pci = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        2,
        shaders,
        &inStateCInfo,
        &inAsmCreateInfo,
        nullptr,
        &vpStateCrInfo,
        &rasterizationStateCreateInfo,
        &multiStateCreateInfo,
        nullptr,
        &cbStateInfo,
        &dStateCreateInfo,
        pipelineLayout,
        renderPass,
        0,
        VK_NULL_HANDLE,
        -1
    };

    if (vkCreateGraphicsPipelines(ctx.dev, VK_NULL_HANDLE, 1, &pci, nullptr, &pl )) {
        ctx.pform.FatalError("Could not create the basic pipeline!", "Vulkan Runtime Error");
    }
    return pl;
}


VkDescriptorSetLayout RasterizationRenderer::DescriptorSetLayoutGatherPass(void) {
    VkDescriptorSetLayout basicLayout;

    Vector<VkDescriptorSetLayoutBinding> bindings(7);

    bindings[0] = { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT }; //matrix
    bindings[1] = { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // light

    bindings[2] = { 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // roughness
    bindings[3] = { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // diffuse
    bindings[4] = { 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; //specular
    bindings[5] = { 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; //normal

    VkDescriptorSetLayoutCreateInfo setCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, (u32)bindings.sz, bindings.data

    };

    if (vkCreateDescriptorSetLayout(ctx.dev, &setCreateInfo, nullptr, &basicLayout) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create basic descriptor set layout", "Vulkan Runtime Error");
    }

    return basicLayout;
}

VkPipelineLayout RasterizationRenderer::PipelineLayoutGatherPass(VkDescriptorSetLayout& dsLayout) {
    VkPipelineLayout plLayout;
    VkPipelineLayoutCreateInfo layoutCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 1, &dsLayout, 0, nullptr
    };

    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(u32);
    pushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;





    layoutCreateInfo.pPushConstantRanges = &pushConstant;
    layoutCreateInfo.pushConstantRangeCount = 1;
    if (vkCreatePipelineLayout(ctx.dev, &layoutCreateInfo, nullptr, &plLayout) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create a basic pipeline layout", "Vulkan Runtime Error");
    }
    return plLayout;
}


VkRenderPass RasterizationRenderer::RenderPassGatherPass(void) {
    VkRenderPass rp;

    VkAttachmentDescription attachmentDescrs[] = { {
        0, VK_FORMAT_D16_UNORM , VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,  VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
    };
    VkAttachmentReference colorReferences[] = { {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} };
    VkSubpassDescription subpassDescriptions[] = { {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, colorReferences, nullptr, nullptr, 0, nullptr} };
    VkRenderPassCreateInfo rpCreateInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0 ,1, attachmentDescrs, 1, subpassDescriptions, 0, nullptr
    };
    if (vkCreateRenderPass(ctx.dev, &rpCreateInfo, nullptr, &rp) != VK_SUCCESS) {
        ctx.pform.FatalError("Could not create render pass", "Vulkan Runtime Error");
    }

    return rp;
}

FBAttachment::FBAttachment(VkFormat format, VkImageUsageFlags flags, u32 w, u32 h, VulkanContext& ctx) {
    VkImageAspectFlags aspectMask = 0;
    VkImageLayout imgLayout;
    this->format = format;

    if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    } else if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        imgLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkImageCreateInfo cInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        format,
        {
            w,
            h,
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

    VkMemoryRequirements memReqs;
    if (vkCreateImage(ctx.dev, &cInfo, nullptr, &this->image) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create a vulkan image", "Vulkan Runtime Error");
    }
    VkPhysicalDeviceMemoryProperties  physProps;
    vkGetPhysicalDeviceMemoryProperties(ctx.gpu, &physProps);
    auto mType = BasicRenderer::GetMemoryTypes(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physProps);
    VkMemoryAllocateInfo memalloc = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memReqs.size,
        mType
    };

    vkAllocateMemory(ctx.dev, &memalloc, nullptr, &this->mem);
    vkBindImageMemory(ctx.dev, this->image, this->mem, 0);


    VkImageViewCreateInfo viewCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        this->image, VK_IMAGE_VIEW_TYPE_2D,
        this->format,
        {
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY
        },
        {
            VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
        }
    };

    vkCreateImageView(ctx.dev, &viewCreateInfo, nullptr, &this->view);
}