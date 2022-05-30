// 05/23/2022

#ifndef DEVICE_H
#define DEVICE_H
#define NUM_IMAGES 3

#include "platform.h"
#include "types.h"
#include "vulkan.h"


struct PerFrameData {
    VkFramebuffer framebuffer;
    VkCommandBuffer commandBuffer;
    VkSemaphore renderingReady;
    VkSemaphore presentReady;
    VkFence fence;
};

struct QueueParams {
	VkQueue handle;
	uint32_t familyIndex; // e.g present or graphics
	QueueParams() {
		handle = VK_NULL_HANDLE;
		familyIndex = 0;
	}
};

struct SwapchainData {
    VkSwapchainKHR handle = VK_NULL_HANDLE;
    VkSurfaceFormatKHR format;
    VkDeviceMemory memory;
};


struct BufferParams {
	VkBuffer handle;
	VkDeviceMemory memory;
	uint32_t size;
	BufferParams() {
		handle = VK_NULL_HANDLE;
		memory = VK_NULL_HANDLE;
		size = 0;
	}
};

struct Image {
    VkImage handle;
    VkImageView view;
    VkSampler sampler;
    VkDeviceMemory mem;
    
};

// (TODO) learn what can be done individually on each thread
struct VulkanContext {
	Platform pform;
	VkDevice device = VK_NULL_HANDLE;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surf = VK_NULL_HANDLE;
    VkPhysicalDevice gpu = VK_NULL_HANDLE;

    VkDevice dev;
    u32 gqFamilyIndex;
    VkQueue graphicsPresentQueue;
    Image images[NUM_IMAGES];
    VkExtent2D ext;
    SwapchainData sc;

    VkCommandPool cmdPool = VK_NULL_HANDLE;
    PerFrameData frameResources[NUM_IMAGES];
    
    
    
	bool debugLayersOn;
    
	VulkanContext() {}
    
	bool LoadExportedFunctions(void);
	bool LoadExportedFns(void);
	bool LoadGlobalFns(void);
	bool LoadInstanceFns(void);
    bool LoadDevFns(void);
	void Device(bool debuglayers);
    void Swapchain(void);
    void CheckDebugLayers(void);
    
    int EnsureSuperset(Vector<VkLayerProperties>& given, Vector<const char*>& wanted);
    bool ExtensionAvail(Vector<VkExtensionProperties>& extensions, const char* extension);
    bool CheckMajor(int version);
    bool CheckProperties(VkPhysicalDeviceProperties& props);
    bool CheckFeatures(VkPhysicalDeviceFeatures& features);
    
    //bool CheckQueues(VkPhysicalDevice& dev);
    
    void Init(void);
    void CreateCmdPool(VkCommandPoolCreateFlags flags);
    void AllocCmdBuffers(void);
    void Semaphores(void);
    
	
    
};

#endif
