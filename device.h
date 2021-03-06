// 05/23/2022

#ifndef DEVICE_H
#define DEVICE_H
#define NUM_IMAGES 3

#include "platform.h"
#include "types.h"
#include "vulkan.h"
#include "utils.h"

struct QueueParams {
	VkQueue handle;
	uint32_t familyIndex; // e.g present or graphics
	QueueParams() {
		handle = VK_NULL_HANDLE;
		familyIndex = 0;
	}
};


struct PerFrameData {
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkSemaphore renderingDone = VK_NULL_HANDLE;
    VkSemaphore presentOK = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
};


struct Image {
    VkImage handle = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    
};


struct SwapchainData {
    VkSwapchainKHR handle = VK_NULL_HANDLE;
    VkSurfaceFormatKHR format;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};



// (TODO) learn what can be done individually on each thread
struct VulkanContext {
	Platform pform;

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surf = VK_NULL_HANDLE;
    VkPhysicalDevice gpu = VK_NULL_HANDLE;

    VkDevice dev = VK_NULL_HANDLE;
    u32 gqFamilyIndex;
    VkQueue graphicsPresentQueue;
    Image images[NUM_IMAGES];
    VkExtent2D ext;
    SwapchainData sc;

    VkCommandPool cmdPool = VK_NULL_HANDLE;
    PerFrameData frameResources[NUM_IMAGES];
    u32 currFrame = 0;
    
    
	bool debugLayersOn;
    
	VulkanContext() {}
    ~VulkanContext();
    
    
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
    void Fences(void);
    void Cleanup(void);
  void WindowChange(void);
    
};

#endif
