// Does the device setup necessary for Vulkan
#include "vulkan.h"

#define VK_EXPORTED_FUNCTION( fun ) PFN_##fun fun;
#define VK_GLOBAL_LEVEL_FUNCTION( fun ) PFN_##fun fun;
#define VK_INSTANCE_LEVEL_FUNCTION( fun ) PFN_##fun fun;
#define VK_DEVICE_LEVEL_FUNCTION( fun ) PFN_##fun fun;
#include "functionslist.h"



#include "device.h"
#include "platform.h"
#include <vector>
#include <string.h>

char [1000] errString;


#define ENSURE_SUCC( expr ) if ( (expr) != VK_SUCCESS) {snprintf(errString, 1000, "Failed to do %s", #expr); FatalError(errString, "Vulkan Error"); }

int clamp(int min, int max, int v) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}


bool VulkanContext::LoadExportedFns(void) {
	#define VK_E_FN( fun ) fun = pform.Wrangle(#fun); if (!fun) { printf("Failed to load %s", #fun); return false;}
	#include "functionslist.h"
	return true;
}

bool VulkanContext::LoadGlobalFns(void) {
	#define VK_G_FN ( fun ) fun = (PFN_##fun)vkGetInstanceLevelProcAddress(nullptr, #fun); if (!fun) {printf("Failed to load %s", #fun); return false;}
	#include "functionslist.h"
	return true;
}

bool VulkanContext::LoadInstanceFns(void) {
	#define VK_I_FN ( fun ) fun = (PFN_##fun)vkGetInstanceLevelProcAddress(instance, #fun); if (!fun) {printf("Failed to load %s", #fun); return false;}
	#include "functionslist.h"
	return true;
}


int VulkanContext::EnsureSuperset(std::vector<VkLayer>& given, std::vector<const char*>& wanted) {

	for (size_t i = 0; i < wanted.size(); i++) {
		bool found = false;
		for (size_t j = 0; j < given.size(); j++) {
			const char* wantedStr = wanted[i];
			const char* givenStr = given[i].layername;
			if (!strcmp(wantedStr, givenStr)) {
				found = true;
				break;
			} 
		}
		if (!found) {
			return static_cast<int>(i);
		}
	}
	return static_cast<int>(given.size());
}



void VulkanContext::CheckDebugLayers(void) {
	std::vector<const char*> neededLayers = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_RENDERDOC_Capture"};
	u32 numLayers;
	vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
	std::vector<VkLayerProperties> vulkanLayers(numLayers);
	vkEnumerateInstanceLayerProperties(&numLayers, vulkanLayers.data());
	
	int okay = EnsureSuperset(vulkanLayers, neededLayers);
	if (okay != 2) {
		okay == 0 ? PopupWarning(wanted[i], "Unable to find validationLayers!") : PopupWarning(wanted[i], "Unable to find renderdoc extension!"); 
	}

}


bool VulkanContext::ExtensionAvail(std::vector<VkExtensionProperties>& extensions, const char* extension) {
    for (u64 i = 0; i < extensions.size(); i++) {
        if (!strcmp(extension, extensions[i].extensionName)) {
            return true;
        }
    }
    return false;
}

bool VulkanContext::CheckMajor(int version) {
    return true;
}

bool VulkanContext::CheckProperties(VkPhysicalDeviceProperties& props) {
    int minor = VK_VERSION_MAJOR(props.apiVersion);
    return CheckMajor(minor);
}


bool VulkanContext::CheckFeatures(VkPhysicalDeviceFeatures& features) {
    return true;
}


bool VulkanContext::CheckQueues(VkPhysicalDevice& dev) {
    u32 numFamilies;
    
}

void VulkanContext::Device(bool validationLayers) {
	if (debugLayers) {
		CheckDebugLayers();
	}

	u32 numAvail, numDevices;
	if (vkEnumerateInstanceExtensionProperties(nullptr, &numAvail, nullptr) != VK_SUCCESS) {
		FatalError("Issue when finding the number of extensions");
	}
	std::vector<const char*> instanceExtensions  = { platformSurfaceExtensionName, surfaceExtensionName}; // one of these doesn't belong
    std::vector<const char*> deviceExtensions = { swapChainExtensionName};
    std::vector<VkExtensionProperties> availableExtensions(&numAvail);
    
    if (vkEnumerateInstanceExtensionProperties(nullptr, &numAvail, &instanceExtensions[0]) != VK_SUCCESS) {
        FatalError("Issue while enumerating extensions!", "Setup Error");
    }


    for (u32 i = 0; i < static_cast<u32>(extensions.size()); i++) {
        if (!ExtensionAvail(availableExtensions, extensions[i])) {
            FatalError("Issue while checking for extensions", "Setup Error");
        }
    }


    VkApplicationInfo applInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "Vulkan Renderer",
        VK_MAKE_VERSION(1, 0, 0),
        "Renderer",
        VK_MAKE_VERSION(1, 0, 0),
        VK_MAKE_VERSION(1, 0, 0)
    };

    VkInstanceCreateInfo instInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0,
        &applInfo,
        0,
        nullptr,
        static_cast<u32> (extensions.size()),
        extensions.data()
    };

    if (debugLayers) {
        instInfo.enabledLayerCount = (u32) layers.size();
        instInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateInstance(&instInfo, nullptr, instance)  != VK_SUCCESS) {
        FatalError("Issue while creating an instance", "Setup Error");
    }

    // Instance created, can ask for the presentation surface
#ifdef _WIN32

    auto surf_info = VkWin32SurfaceCreateInfoKHR = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, pform.window.Instance,
        pform.window.inst
    };

    
    ENSURE_SUCC(vkCreateWin32SurfaceKHR(instance, &surf_info, nullptr, &surf));
#endif

    ENSURE_SUCC( vkEnumeratePhysicalDevices(instance, numDevices, nullptr));
    std::vector<VkPhysicalDevice> physDevs(numDevices);
    ENSURE_SUCC (vkEnumeratePhysicalDevices(instance, &numDevices, &physDevs));
    // Loop over with all of the devices to see if an of them are good
    bool found = 0;
    u32 graphicsQueueFamilyIndex = 0xFFFFFFFF;
    
    for (u32 i  = 0; i < numDevices; i++) {
        // Check if the device is OK
        u32 numPhysExtensions;
        auto& dev = physDevs[i];
        if (vkEnumerateExtensionProperties (dev, nullptr, &numPhysExtensions, nullptr) != VK_SUCCESS) continue;
        std::vector<VkExtensionProperties> availExtensions(numPhysExtensions);
        if (vkEnumerateExtensionProperties (dev, nullptr, &numPhysExtensions, &availExtensions[0]) != VK_SUCCESS) continue;

        for (u32 j = 0; j < deviceExtensions.size(); j++) {
            if ( !ExtensionAvail(  availExtensions, deviceExtensions[j] ) ) {
                continue;
            }
        }
        
        // check for properies and features
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;

        vkGetPhysicalDeviceProperties(dev, props);
        vkGetPhysicalDeviceFeatures(dev, features);
        if (!CheckProperties(props))
            continue;
        if (!CheckFeatures(props)) {
            continue;
        }

        u32 qFamilies;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &qFamilies, nullptr);
        if (qFamilies == 0) {
            continue;
        }

        std::vector<VkQueueFamilyProperties> props(qFamilies);
        std::vector<VkBool32> canPresent(qFamilies);

        vkGetPhysicalDeviceQueueFamilyProperties(dev, &qFamilies, &props[0]);
        


        for (u32 j = 0; j < qFamilies; j++) {
            vkGetPhysicalDeviceSupportKHR(dev, j, surf, &canPresent[i]);
        }
        
        for (u32 j = 0; j < qFamilies; j++) {
            if ((props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                if (canPresent[i] == VK_TRUE) {
                    graphicsQueueFamilyIndex = j;
                    gpu = dev;
                }
            }
        }
        
    }
    if (gpu == VK_NULL_HANDLE) {
        FatalError("Unable to find a suitable physical device", "Vulkan Error");
    }
    gqFamilyIndex = graphicsQueueFamilyIndex;
    f32 qPriorities = 1.0f;

    VkDeviceQueueCreateInfo qCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr, 0, graphicsQueueFamilyIndex, 1, qPriorities
    };

    VkDeviceCreateInfo devCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1,
        &qCreateInfo, 0, nullptr, (u32) deviceExtensions.size(),
        &deviceExtensions[0], nullptr
    };

    if (vkCreateDevice(gpu, &devCreateInfo, nullptr, this->dev) != VK_SUCCESS) {
        FatalError("Unable to create the device!", "Vulkan Setup Error");
    }

    

        
}

/* Create a swapchain, either at init or "on the fly" */
void VulkanContext::Swapchain(void) {
    pform.window.renderable = false;
    VkSurfaceCapabilitiesKHR sCaps;
    vkDeviceWaitIdle(dev);

    for (int i = 0; i < NUM_IMAGES; i++ ) {
        if (images[i].handle != VK_NULL_HANDLE){
            vkDestroyImageView(dev, images[i].view, nullptr);
            images[i].view = VK_NULL_HANDLE;
        }
    }

    auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surf, &sCaps);

    if (res != VK_SUCCESS) {
        FatalError("Could not check for surface presentation!", "Vulkan Swapchain Creation Error");
    }

    u32 formatsCount;
    ENSURE_SUCC( vkGetPhysicalDeviceSurfaceFormatsKHR (gpu, surf, &formatsCount, nullptr));
    if (formatsCount == 0) FatalError("Unable to enumerate formats", "Vulkan Swapchain Creation Error");

    std::vector<VkSurfaceFormatKHR> formats(formatsCount);
    ENSURE_SUCC( vkGetPhysicalDeviceSurfaceFormatsKHR (gpu, surf, &formatsCount, &formats[0]));

    u32 ct;
    ENSURE_SUCC ( vkGetPhysicalDeviceSurfacePresentModesKHR (gpu, surf, &ct, nullptr) );
    std::vector<VkPresentModeKHR> presentModes(ct);
    ENSURE_SUCC ( vkGetPhysicalDeviceSurfacePresentModesKHR (gpu, surf, &ct, &presentModes[0]) );
    
    if (NUM_IMAGES > sCaps.maxImageCount || NUM_IMAGES < sCaps.minImageCount) {
        FatalError("Could not get number of swapchain images", "Vulkan Swapchain Creation Error");
    }

    
    // Go through and choose the best format
    VkSurfaceFormatKHR wantedFormat;
    bool chosen = false; 
    for (size_t i = 0; i < formats.size() && !chosen; i++) {
        if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM) {
            wantedFormat = formats[i];
            chosen = true;
        }
    }
    if (!chosen) {
        wantedFormat = formats[0].format == VK_FORMAT_UNDEFINED ? {VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR} : formats[i];
    }

    int w, h = pform.GetRect(&w, &h);
    ext = { clamp(sCaps.minImageExtent.width, sCaps.maxImageExtent.width,w ),
        clamp(ext.sCaps.minImageExtent.height, ext.sCaps.maxImageExtent.height, h)};

    VkImageUsageFlags flags =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (caps->supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        FatalError("Unable to get proper flags", "Vulkan Swapchain Creation Error");
    }

    VkSurfaceTransformFlagBitsKHR 
    
    
    
    
}



