// 05/23/2022

#define NUM_IMAGES 3

struct QueueParams {
	VkQueue handle;
	uint32_t familyIndex; // e.g present or graphics
	QueueParams() {
		handle = VK_NULL_HANDLE;
		familyIndex = 0;
	}
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
    vkSampler sampler;
    VkDeviceMemory mem;

};

struct VulkanContext {
	Platform pform;
	VkDevice device = VK_NULL_HANDLE;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surf = VK_NULL_HANDLE;
    VkPhysicalDevice gpu = VK_NULL_HANDLE;
    VkDevice dev;
    u32 gqFamilyIndex;
    Images images[NUM_IMAGES];
    VKExtent2D ext;



	bool debugLayers;

	VulkanContext();

	bool LoadExportedFunctions(void);
	bool LoadExportedFns(void);
	bool LoadGlobalFns(void);
	bool LoadInstanceFns(void);
	void Device(bool debuglayers);
    void Swapchain(void);
    void RecreateSwapchain(void);


	

};





