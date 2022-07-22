
#include "rasterization_renderer.h"

VkDescriptorPool RasterizationRenderer::DescriptorPool(u32 nDescriptors) {
    VkDescriptorPool pool;
    Vector<VkDescriptorPoolSize> szes(6);
    szes[0] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors };
    szes[1] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors };
    szes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors };
    szes[3] = { VK_DESCRIPTOR_TYPE_}

    VkDescriptorPoolCreateInfo cInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, nDescriptors, (u32)szes.sz, szes.data };
    if (vkCreateDescriptorPool(ctx.dev, &cInfo, nullptr, &pool) != VK_SUCCESS) {
        ctx.pform.FatalError("Could not create descriptor pool for basic shader", "Vulkan Runtime Error");
    }
    return pool;
}