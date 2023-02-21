# Classes


## **BasicRenderer**

### Members

`VulkanContext ctx` - contains the Vulkan context for the renderer

`Buffer stagingBuffer` - contains a staging buffer that is reused for separate transfers from **CPU-accessible GPU memory** to **GPU-exclusive memory**. This buffer therefore has the **host visible** property. To actually move data to the **GPU-exclusive memory**, you need to create a command buffer and make a call to `vkCmdCopyBuffer`, then follow it with a pipeline barrier

`BasicRenderData rData` - This contains the **descriptor set**, the **descriptor set layout**, the **pipeline layout object**, the **pipeline**, and one **renderpass**. It looks like this memory is currenty **not** released, and it probably should be. 


`Buffer uniformBasicHostPool` - This is, like the staging buffer, a buffer that has **CPU-accessible GPU memory** and is used to transfer to **GPU-exclusive memory**. It contains data for uniforms for objects, such as matrices. It looks like this memory is currenty **not** released, and it probably should be. 

`Buffer uniformBasicMegaPool` - This is the **GPU-exclusive memory** target for the `uniformBasicHostPool`

`Buffer uniformBasicHostLightPool` - This is, like the staging buffer, a buffer that has **CPU-accessible GPU memory** and is used to transfer to **GPU-exclusive memory**. It contains data for uniforms for lights, such as position and direction. It looks like this memory is currenty **not** released, and it probably should be. 

`Buffer uniformBasicMegaLightPool` - This is the **GPU-exclusive memory** target for the `uniformBasicHostLightPool`


`VkDescriptorPool basicDescriptorPool` - Descriptors are allocated from a pool. Once the pool is made, it is possible to create a descriptor for each model. Each model has its own descriptor set because it is probably something that is needed in a command buffer, so it cannot be changed and reused in the draw loop. The descriptor set is updated with a call to `vkUpdateDescriptorSets`, which is NOT executed in a command buffer

`VkDescriptorSetLayout basicLayout` - each descriptor set needs a layout. This corresponds to the layout with a uniform in the vertex shader (matrices), a uniform in the fragment shader (bunch of lights), and a texture in the fragment shader (texture). Each model uses the same lights, and each model has a unique offset into the matrix buffer included when the descriptors are written:

    Vector<VkDescriptorSetLayoutBinding> bindings(3);
    bindings[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT};
    bindings[1] = {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT};
    bindings[2] = {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT};

`bool uniformPoolNeedsCopy = true` - If the matrices and the lights need to be updated in their big mega buffer, then this will be true. This copying will come AFTER the main loop has started recording and will slot a pipeline barrier after the commands to do the copied, which probably make this a lot slower.

`bool lbOK = false` - unused

`bool fenceSet[3]` - the swapchain consists of 3 sets of frame data.

`bool sceneNeedsUpdate = true` - whether the scene needs to be updating


### Synchronization

There are 3 synchronization primitives that are used in the main loop:
    
    VkSemaphore renderingDone = VK_NULL_HANDLE;
    VkSemaphore presentOK = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE; 

- The function will not present until the frame is done being rendered (rather basic): 
  
  VkPresentInfoKHR pInfo = {

      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, 1,
      &pfd->renderingDone, 1, &ctx.sc.handle,
      &imgIndex, nullptr
    };

    res = vkQueuePresentKHR(ctx.graphicsPresentQueue, &pInfo);

- The function will not start drawing/rendering until it has acquired the image (which should not happen until the previous frame is done with presenting):

  vkAcquireNextImageKHR(ctx.dev, ctx.sc.handle, UINT64_MAX,
                                     **pfd->presentOK**, VK_NULL_HANDLE, &imgIndex);
  
  ...

  VkSubmitInfo sInfo = {

      VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1,
      &pfd->presentOK, &v,
      1, &pfd->commandBuffer,1, &pfd->renderingDone
    };

    if (vkQueueSubmit(ctx.graphicsPresentQueue, 1, &sInfo, pfd->fence) != VK_SUCCESS) {

      return;
    }

- The function will prevent a bunch of command buffers from being recorded through use of a fence


## Execution Barriers

- Use vkCmdPipelineBarrier() to accomplish this. All commands PAST the barrier at dstStage will have to wait for all commands BEFORE the barrier to complete srcStage

## Memory Barriers

- Vulkan needs memory barriers to establish coherency. When the L2 cache contains the most up-to-date data there is, we can say that the memory is available. Memory is visible to {STAGE, ACCESS_COMBINATION} if memory is made available and we make that memory visible to that {STAGE, ACCESS_COMBINATION}. Making memory available will flush the caches, and making memory visible will invalidate caches. So the stage and access combination are different. As an example, if we run vkCmdPipelineBarrier(), we get:
    1. Wait for srcStageMask to complete
    2. Make all writes performed in possible combinations of {srcStageMask, srcAccessMask} available
    3. Make available memory from step 2 visible to possible combinations of {dstStageMask, dstAccessMask}
    4. Unblock work in dstStageMask 

## ImageMemoryBarrier

- You need these to change layouts. Layout transitions happen in-between {make available} and {make visible} stages of a memory barrier. It makes memory that was available (e.g in L2 memory) into memory that is also available (e.g in L2 cache), in a different state

### Functions


`Buffer MakeBuffer(u32 sizeIn, u32 flagsIn,  VkMemoryPropertyFlagBits wantedProperty)`. This returns a Buffer object, which has a ptr to the context, has a handle for the API, and it has a memory handle.

`Buffer StagingBuffer( u32 sz, void* data)`. This will make a staging buffer with the data, or if the one from before has enough space, then it will make the already existing buffer the target of the transfer. To make a buffer, you need memory as well as a buffer, then you will bind the buffer to the memory with `vkBindBufferMemory()`

`Buffer VertexBuffer( u32 sz, void* data)`. This will make a staging buffer with need be, then it will move the data from the staging buffer to the GPU-exclusive memory 

`void toGPU(void* from, VkDeviceMemory mem, u32 sz )`. This will move the data from the pointer given to the memory by mapping the GPU memory so it is accessible by the CPU. Options have to be specified beforehand as not all GPU-memory is accessible

`Texture MakeTexture(const char* fileName, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, u32 numComps = 4)`. Loads an image from memory, creates a VkImage, and fills the texture. In order to fill the texture, you need an image view, a sampler, record a command buffer with a call to `vkCmdCopyBufferToImage` and submit it to a queue. The queue needs an execution barrier ( Transfer -> Fragment ) and a memory barrier (Transfer -> Shader Read). Double check that this is the case that we want Transfer -> Fragment for the execution barrier. 