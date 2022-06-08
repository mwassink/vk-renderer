
#if !defined(VK_E_FN)
#define VK_E_FN( fun )
#endif

VK_E_FN( vkGetInstanceProcAddr )

#undef VK_E_FN

#if !defined(VK_G_FN)
#define VK_G_FN( fun )
#endif

VK_G_FN( vkCreateInstance )
VK_G_FN( vkEnumerateInstanceExtensionProperties )
VK_G_FN( vkEnumerateInstanceLayerProperties )

#undef VK_G_FN

#if !defined(VK_IN_FN)
#define VK_IN_FN( fun )
#endif

#if defined(USE_SWAPCHAIN_EXTENSIONS)

VK_IN_FN( vkGetPhysicalDeviceSurfaceSupportKHR )
VK_IN_FN( vkGetPhysicalDeviceSurfaceCapabilitiesKHR )
VK_IN_FN( vkGetPhysicalDeviceSurfaceFormatsKHR )
VK_IN_FN( vkGetPhysicalDeviceSurfacePresentModesKHR )
VK_IN_FN( vkDestroySurfaceKHR )
VK_IN_FN( vkCreateWin32SurfaceKHR )
#endif

VK_IN_FN( vkDestroyInstance )
VK_IN_FN( vkEnumeratePhysicalDevices )
VK_IN_FN( vkGetPhysicalDeviceProperties )
VK_IN_FN( vkGetPhysicalDeviceFeatures )
VK_IN_FN( vkGetPhysicalDeviceQueueFamilyProperties )
VK_IN_FN( vkCreateDevice )
VK_IN_FN( vkGetDeviceProcAddr )
VK_IN_FN( vkEnumerateDeviceExtensionProperties )
VK_IN_FN( vkGetPhysicalDeviceMemoryProperties )
#undef VK_IN_FN

#if !defined(VK_D_FN)
#define VK_D_FN( fun )
#endif

VK_D_FN(vkGetDeviceQueue)
VK_D_FN(vkDestroyDevice)
VK_D_FN(vkDeviceWaitIdle)

VK_D_FN( vkCreateSemaphore )
VK_D_FN( vkCreateCommandPool )
VK_D_FN( vkAllocateCommandBuffers )
VK_D_FN( vkBeginCommandBuffer )
VK_D_FN( vkCmdPipelineBarrier )
VK_D_FN( vkCmdClearColorImage )
VK_D_FN( vkEndCommandBuffer )
VK_D_FN( vkQueueSubmit )
VK_D_FN( vkFreeCommandBuffers )
VK_D_FN( vkDestroyCommandPool )
VK_D_FN( vkDestroySemaphore )
VK_D_FN( vkCreateBuffer )
VK_D_FN( vkGetBufferMemoryRequirements )

#if defined(USE_SWAPCHAIN_EXTENSIONS)
VK_D_FN( vkCreateSwapchainKHR )
VK_D_FN( vkGetSwapchainImagesKHR )
VK_D_FN( vkAcquireNextImageKHR )
VK_D_FN( vkQueuePresentKHR )
VK_D_FN( vkDestroySwapchainKHR )
#endif

VK_D_FN( vkCreateImage )
VK_D_FN( vkCreateImageView )
VK_D_FN( vkCreateRenderPass )
VK_D_FN( vkCreateFramebuffer )
VK_D_FN( vkCreateShaderModule )
VK_D_FN( vkCreatePipelineLayout )
VK_D_FN( vkCreateGraphicsPipelines )
VK_D_FN( vkCmdBeginRenderPass )
VK_D_FN( vkCmdBindPipeline )
VK_D_FN( vkCmdDraw )
VK_D_FN( vkCmdEndRenderPass )
VK_D_FN( vkDestroyShaderModule )
VK_D_FN( vkDestroyPipelineLayout )
VK_D_FN( vkDestroyPipeline )
VK_D_FN( vkDestroyRenderPass )
VK_D_FN( vkDestroyFramebuffer )
VK_D_FN( vkDestroyImageView )


VK_D_FN(vkCreateFence)
VK_D_FN(vkAllocateMemory)
VK_D_FN(vkBindBufferMemory)
VK_D_FN(vkMapMemory)
VK_D_FN(vkFlushMappedMemoryRanges)
VK_D_FN(vkUnmapMemory)
VK_D_FN(vkCmdSetViewport)
VK_D_FN(vkCmdSetScissor)
VK_D_FN(vkCmdBindVertexBuffers)
VK_D_FN(vkWaitForFences)
VK_D_FN(vkResetFences)
VK_D_FN(vkFreeMemory)
VK_D_FN(vkDestroyBuffer)
VK_D_FN(vkDestroyFence)



#undef VK_D_FN
