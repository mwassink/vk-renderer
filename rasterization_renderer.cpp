
#include "rasterization_renderer.h"

VkDescriptorPool RasterizationRenderer::DescriptorPool(u32 nDescriptors) {
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


VkPipeline RasterizationRenderer::ScenePipeline(void) {

    
    auto vMod = ShaderModule("shaders/Model.vert");
    auto pMod = ShaderModule("shaders/Model.frag");

    VkPipeline pl;

    VkPipelineShaderStageCreateInfo shaders[] ={
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vMod, "main", nullptr},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, pMod, "main", nullptr}
    };

    VkVertexInputBindingDescription bindingDescription = {
        0, sizeof(Vertex)
    };
    auto binding = bindingDescription.binding;
    VkVertexInputAttributeDescription vertexAttrDescrs [] = {
        {0, binding, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        {1, binding, VK_FORMAT_R32G32_SFLOAT, 4 * sizeof(f32)},
        {2, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
        {3, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)}        
    };

    VkPipelineVertexInputStateCreateInfo inStateCInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &bindingDescription, 4, vertexAttrDescrs
    };

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

    VkPipelineColorBlendAttachmentState cBlendState = {
        VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT |VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT  
    };

    VkPipelineColorBlendStateCreateInfo cbStateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 1, &cBlendState, {0, 0, 0, 0}
    };

    VkDynamicState dState[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr,
        0, 2, dStates};

    VkGraphicsPipelineCreateInfo pci = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        2,
        &inStateCInfo,
        &inAsmCreateInfo,
        nullptr,
        &vpStateCrInfo,
        &rasterizationStateCreateInfo,
        &multiStateCreateInfo,
        nullptr,
        &cbStateInfo,
        &dStateCreateInfo,
        sceneLayout,
        sceneRenderPass,
        0,
        VK_NULL_HANDLE,
        -1
    };

    if (vkCreateGraphicsPipelines(ctx.dev, VK_NULL_HANDLE, 1, &pci, nullptr, &pl )) {
        ctx.pform.FatalError("Could not create the basic pipeline!", "Vulkan Runtime Error");
    }
    return pl;
}

