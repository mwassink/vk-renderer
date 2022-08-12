
#include "rasterization_renderer.h"

VkDescriptorPool RasterizationRenderer::DescriptorPoolGatherPass(u32 nDescriptors) {
    VkDescriptorPool pool;
    Vector<VkDescriptorPoolSize> szes(5);
    szes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors }; //matrix
    szes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors}; //roughness
    szes[2] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors}; //diffuse
    szes[3] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors}; //normal
    szes[4] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors}; //specular
    
    VkDescriptorPoolCreateInfo cInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, nDescriptors, (u32)szes.sz, szes.data };
    if (vkCreateDescriptorPool(ctx.dev, &cInfo, nullptr, &pool) != VK_SUCCESS) {
        ctx.pform.FatalError("Could not create descriptor pool for basic shader", "Vulkan Runtime Error");
    }
    return pool;
}


VkPipeline RasterizationRenderer::Pipeline(u32 mode, GBufferAttachments& attachments) {

    
    VkShaderModule vMod; 
    VkShaderModule pMod;
    VkPipelineShaderStageCreateInfo shaders[2];
    VkPipelineVertexInputStateCreateInfo inStateCInfo;



    renderPassGather = RenderPassGatherPass(attachments);
    VkDescriptorSetLayout dsLayoutGather = DescriptorSetLayoutGatherPass();
    VkPipelineLayout plLayoutGather = PipelineLayoutGatherPass(dsLayoutGather);
    //VkDescriptorSetLayout dsLayoutDraw = 
    //VkPipelineLayout plLayoutDraw = PipelineLayoutDrawPass()

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
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkDynamicState dState[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 2, dState };




    if (mode == 0) {
        vMod = ShaderModule("shaders/deferred/Model.vert");
        pMod = ShaderModule("shaders/deferred/Model.frag");

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
            {3, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)},
        };

        VkPipelineVertexInputStateCreateInfo tmp = {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &bindingDescription, 4, vertexAttrDescrs
        };
        inStateCInfo = tmp;

        VkPipelineColorBlendAttachmentState cBlendStates[numColorAttachments];
        for (int i = 0; i < numColorAttachments; i++) cBlendStates[i] = cBlendState;

        VkPipelineColorBlendStateCreateInfo cbStateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 5, cBlendStates, {0, 0, 0, 0}
        };



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
            plLayoutGather,
            renderPassGather,
            0,
            VK_NULL_HANDLE,
            -1
        };

        if (vkCreateGraphicsPipelines(ctx.dev, VK_NULL_HANDLE, 1, &pci, nullptr, &pl)) {
            ctx.pform.FatalError("Could not create the basic pipeline!", "Vulkan Runtime Error");
        }


    }
    else if (mode == 1) {
        vMod = ShaderModule("shaders/deferred/Shading.vert");
        pMod = ShaderModule("shaders/deferred/Shading.frag");

        shaders[0] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vMod, "main", nullptr };
        shaders[1] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, pMod, "main", nullptr };

        VkVertexInputBindingDescription bindingDescription = {
            0, sizeof(Vector3)
        };
        u32 binding = bindingDescription.binding;
        VkVertexInputAttributeDescription vertexAttrDescrs[] = {
            {0, binding, VK_FORMAT_R32G32B32_SFLOAT, 0}
        };

        VkPipelineVertexInputStateCreateInfo tmp = {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &bindingDescription, 1, vertexAttrDescrs
        };
        inStateCInfo = tmp;


        VkPipelineColorBlendStateCreateInfo cbStateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 1, &cBlendState, {0, 0, 0, 0}
        };



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
            ,
            ,
            0,
            VK_NULL_HANDLE,
            -1
        };

        if (vkCreateGraphicsPipelines(ctx.dev, VK_NULL_HANDLE, 1, &pci, nullptr, &pl)) {
            ctx.pform.FatalError("Could not create the basic pipeline!", "Vulkan Runtime Error");
        }
    }
    else {
        ctx.pform.FatalError("Unknown mode for pipeline", "Vulkan Runtime Error");
    }

    return pl;
}


VkDescriptorSetLayout RasterizationRenderer::DescriptorSetLayoutGatherPass(void) {
    VkDescriptorSetLayout basicLayout;

    Vector<VkDescriptorSetLayoutBinding> bindings(5);

    bindings[0] = { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT }; //matrix
    bindings[1] = { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // roughness
    bindings[2] = { 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // diffuse
    bindings[3] = { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // normal
    bindings[4] = { 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // specular
    


    
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

    VkPushConstantRange pushConstants[2];
    pushConstants[0].offset = 0;
    pushConstants[0].size = sizeof(u32);
    pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pushConstants[1].offset = 1;
    pushConstants[1].size = sizeof(PushGatherFrag);
    pushConstants[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    layoutCreateInfo.pPushConstantRanges = pushConstants;
    layoutCreateInfo.pushConstantRangeCount = 2;
    if (vkCreatePipelineLayout(ctx.dev, &layoutCreateInfo, nullptr, &plLayout) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create a basic pipeline layout", "Vulkan Runtime Error");
    }
    return plLayout;
}


VkRenderPass RasterizationRenderer::RenderPassGatherPass(GBufferAttachments& attachments) {
    VkRenderPass rp;
    VkAttachmentDescription attachmentDescrs[totalAttachments];

    for (int i = 0; i < totalAttachments; i++) {
        attachmentDescrs[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescrs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescrs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescrs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescrs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        if (i == numColorAttachments)
        {
            attachmentDescrs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescrs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else
        {
            attachmentDescrs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescrs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        attachmentDescrs[i].format = attachments.attachments[i].format;
    }

    
    VkAttachmentReference cReferences[numColorAttachments];
    VkAttachmentReference dReference;

    for (int i = 0; i < numColorAttachments; i++) {        
        cReferences[i].attachment = i;
        cReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    dReference.attachment = numColorAttachments;
    dReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sp = {};
    sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.pColorAttachments = cReferences;
    sp.colorAttachmentCount = numColorAttachments;
    sp.pDepthStencilAttachment = &dReference;

    //The effect is all the commands in the source scope have to finish at least the srcStageMask stage in their execution,
    // before any of the commands in the destination scope are even allowed to start the dstStageMask stage of their execution.

    // this will do an image layout transition:

    //Image subresources can be transitioned from one layout to another as part of a memory dependency (e.g. by using an image memory barrier). 

    VkSubpassDependency dependencies[2] = {
        {
            VK_SUBPASS_EXTERNAL,
            0,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT,
        },
        {
            0,
            VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT,
        }
    };


    VkRenderPassCreateInfo rpCreateInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0 ,1, attachmentDescrs, 1, &sp, 0, nullptr
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
    vkGetImageMemoryRequirements(ctx.dev, this->image, &memReqs);
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

void RasterizationRenderer::CreateAttachments(GBufferAttachments* attachments, u32 w, u32 h) {
    VkFormat depthFormat = GetDepthFormat();
    attachments->diffuseColor = FBAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, w, h, ctx);
    attachments->f0Out = FBAttachment(VK_FORMAT_R16G16B16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, w, h, ctx);
    attachments->normals = FBAttachment(VK_FORMAT_R16G16B16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, w, h, ctx);
    attachments->specularColor = FBAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, w, h, ctx);
    attachments->roughness = FBAttachment(VK_FORMAT_R16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, w, h, ctx);
    attachments->depth = FBAttachment(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, w, h, ctx);
}

void RasterizationRenderer::Init() {
    
    CreateAttachments(&gBufferAttachments, ctx.ext.width, ctx.ext.height);
    pipelineGather = Pipeline(0, gBufferAttachments);

    VkImageView attachments[totalAttachments];
    for (int i = 0; i < totalAttachments; i++) attachments[i] = gBufferAttachments.attachments[i].view;

    VkFramebufferCreateInfo fbCreateInfo = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        nullptr,
        0,
        renderPassGather,
        totalAttachments,
        attachments,
        ctx.ext.width,
        ctx.ext.height,
        1,
    };

    VkSamplerCreateInfo samplerCreateInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0 , VK_FILTER_NEAREST, VK_FILTER_NEAREST,
        VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        0.0f, VK_FALSE, 1.0f, VK_FALSE, VK_COMPARE_OP_ALWAYS, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_FALSE
    };

    if (vkCreateSampler(ctx.dev, &samplerCreateInfo, nullptr, &colorSampler) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to make sampler!", "Vulkan Runtime Error");
    }


}


// Question: how do I feed in the results of one pass as samplers into the next
// need to write a descriptor set, right?

VkDescriptorSetLayout RasterizationRenderer::DescriptorSetLayoutDraw() {

    VkDescriptorSetLayout drawLayout;

    Vector<VkDescriptorSetLayoutBinding> bindings(8);

    bindings[0] = { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT }; //matrix
    bindings[1] = { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // light

    bindings[2] = { 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,   VK_SHADER_STAGE_FRAGMENT_BIT }; // f0
    bindings[3] = { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // roughness
    bindings[4] = { 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // diffuse
    bindings[5] = { 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; //specular
    bindings[6] = { 6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; //normal
    bindings[7] = { 7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }; // pos




    VkDescriptorSetLayoutCreateInfo setCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, (u32)bindings.sz, bindings.data

    };

    if (vkCreateDescriptorSetLayout(ctx.dev, &setCreateInfo, nullptr, &basicLayout) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create basic descriptor set layout", "Vulkan Runtime Error");
    }

    return basicLayout;


    
}

VkDescriptorPool RasterizationRenderer::DescriptorPoolDraw(u32 nDescriptors) {

    VkDescriptorPool pool;
    Vector<VkDescriptorPoolSize> szes(8);
    szes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors }; // matrix
    szes[1] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors }; // light
    szes[2] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors }; // f0tex
    szes[3] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors }; //roughness
    szes[4] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors }; //diffuse
    szes[5] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors }; //normal
    szes[6] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors }; //specular
    szes[7] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors }; // pos


    VkDescriptorPoolCreateInfo cInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, nDescriptors, (u32)szes.sz, szes.data };
    if (vkCreateDescriptorPool(ctx.dev, &cInfo, nullptr, &pool) != VK_SUCCESS) {
        ctx.pform.FatalError("Could not create descriptor pool for basic shader", "Vulkan Runtime Error");
    }
    return pool;
}

VkPipelineLayout RasterizationRenderer::PipelineLayoutDraw(VkDescriptorSetLayout& dsLayout) {


    VkPipelineLayout plLayout;
    VkPipelineLayoutCreateInfo layoutCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 1, &dsLayout, 0, nullptr
    };
    VkPushConstantRange pushConstants[1];
    pushConstants[0].offset = 0;
    pushConstants[0].size = sizeof(u32);
    pushConstants[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    layoutCreateInfo.pPushConstantRanges = pushConstants;
    layoutCreateInfo.pushConstantRangeCount = 1;
    if (vkCreatePipelineLayout(ctx.dev, &layoutCreateInfo, nullptr, &plLayout) != VK_SUCCESS) {
        ctx.pform.FatalError("Unable to create a basic pipeline layout", "Vulkan Runtime Error");
    }
    return plLayout;




}

void RasterizationRenderer::WriteDescriptorSets(VkDescriptorSet& ds, Buffer* buffers, u32* sizes, Texture* textures  ) {

    const int buffersNum = 1;
    const int numImages = 4;
    // loop over the buffers and make writes for all of them
    VkWriteDescriptorSet writes[buffersNum + numImages];
    for (int i = 0; i < buffersNum; i++) {
        VkDescriptorBufferInfo uniforms = { uniformBasicMegaLightPool.handle, 0, sizes[i] };
        writes[i] = DescriptorSetWrite(ds, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, i, &uniforms, true);
    }

    for (int i = 0; i < numImages; i++) {
        VkDescriptorImageInfo imgInfo = { textures[i].sampler, textures[i].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        writes[i + buffersNum] = DescriptorSetWrite(ds, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i + buffersNum, &imgInfo, false);
    }
}



void RasterizationRenderer::BuildScene(Scene* scene) {


}



void RasterizationRenderer::RenderDirect(Scene* scene) {

}