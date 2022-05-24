// 05/23/2022


struct QueueParams {
	VkQueue handle;
	uint32_t familyIndex; // e.g present or graphics
	QueueParams() {
		handle = VK_NULL_HANDLE;
		familyIndex = 0;
	}
};

struct ImageParams {
	VkImage imageHandle;
	VkImageView imageView;
	VkSampler sampler;
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


struct VulkanContext {

};



struct VulkanDevice {
	Platform pform;
	VkDevice device;
	bool LoadExportedFunctions(void);
	bool LoadExportedFns(void);
	bool LoadGlobalFns(void);
	bool LoadInstanceFns(void);

	

};





