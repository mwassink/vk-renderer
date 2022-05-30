// Does the device setup necessary for Vulkan
#include "vulkan.h"

#define VK_E_FN( fun ) PFN_##fun fun;
#define VK_G_FN( fun ) PFN_##fun fun;
#define VK_IN_FN( fun ) PFN_##fun fun;
#define VK_D_FN( fun ) PFN_##fun fun;
#include "functionslist.h"



#include "device.h"
#include "platform.h"
#include "types.h"
#include <string.h>
#include <stdio.h>
char errString[1000];


#define ENSURE_SUCC( expr ) if ( (expr) != VK_SUCCESS) {snprintf(errString, 1000, "Failed to do %s", #expr); pform.FatalError(errString, "Vulkan Error"); }

int clamp(int min, int max, int v) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}


u32 clamp(u32 min, u32 max, u32 v) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}



bool VulkanContext::LoadExportedFns(void) {
#define VK_E_FN( fun ) fun = (PFN_##fun) pform.WrangleExportedEntry(#fun); if (!fun) { snprintf(errString, 1000, "Failed to load %s", #fun); return false;}
#include "functionslist.h"
	return true;
}

bool VulkanContext::LoadGlobalFns(void) {
#define VK_G_FN( fun ) fun = (PFN_##fun)vkGetInstanceProcAddr(nullptr, #fun); if (!fun) {snprintf(errString,1000,  "Failed to load %s", #fun); return false;}
#include "functionslist.h"
	return true;
}

bool VulkanContext::LoadInstanceFns(void) {
#define VK_IN_FN( fun ) fun = (PFN_##fun)vkGetInstanceProcAddr(instance, #fun); if (!fun) {snprintf(errString, 1000, "Failed to load %s", #fun); return false;}
#include "functionslist.h"
	return true;
}

bool VulkanContext::LoadDevFns(void) {
    //int ctr = 0;
#define VK_D_FN( fun ) fun = (PFN_##fun)vkGetDeviceProcAddr(dev, #fun); if (!fun) \
{snprintf(errString, 1000, "Failed to load %s", #fun); return false;} //else {ctr++; }
#include "functionslist.h"
    
    //sprintf(errString, "Loaded %d extensions!", ctr);
    // pform.PopupWarning(errString, "Message");
    return true;
}


int VulkanContext::EnsureSuperset(Vector<VkLayerProperties>& given, Vector<const char*>& wanted) {
    
	for (size_t i = 0; i < wanted.size(); i++) {
		bool found = false;
		for (size_t j = 0; j < given.size(); j++) {
			const char* wantedStr = wanted[i];
			const char* givenStr = given[i].layerName;
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
	Vector<const char*> neededLayers(2);
    neededLayers[0] = "VK_LAYER_KHRONOS_validation";
    neededLayers[1] ="VK_LAYER_RENDERDOC_Capture"; 
	u32 numLayers;
	vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
	Vector<VkLayerProperties> vulkanLayers(numLayers);
	vkEnumerateInstanceLayerProperties(&numLayers, vulkanLayers.data);
	
	int okay = EnsureSuperset(vulkanLayers, neededLayers);
	if (okay != 2) {
		okay == 0 ? pform.PopupWarning(neededLayers[0], "Unable to find validationLayers!") : pform.PopupWarning(neededLayers[1], "Unable to find renderdoc extension!"); 
	}
    
}


bool VulkanContext::ExtensionAvail(Vector<VkExtensionProperties>& extensions, const char* extension) {
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



void VulkanContext::Device(bool validationLayers) {
    Vector<const char*> debugLayers(2);
    debugLayers[0] = "VK_LAYER_KHRONOS_validation";
    debugLayers[1] = "VK_LAYER_RENDERDOC_Capture";
    
    if (validationLayers) {
        CheckDebugLayers();
    }
    
    u32 numAvail, numDevices;
    if (vkEnumerateInstanceExtensionProperties(nullptr, &numAvail, nullptr) != VK_SUCCESS) {
        pform.FatalError("Issue when finding the number of extensions", "Setup Error");
    }
    Vector<const char*> instanceExtensions(2);
    instanceExtensions[0]= platformSurfaceExtensionName; // one of these doesn't belong
    instanceExtensions[1] = surfaceExtensionName;
    Vector<const char*> deviceExtensions(1);
    deviceExtensions[0] = swapChainExtensionName;
    Vector<VkExtensionProperties> availableExtensions(numAvail);
    
    if (vkEnumerateInstanceExtensionProperties(nullptr, &numAvail, &availableExtensions[0]) != VK_SUCCESS) {
        pform.FatalError("Issue while enumerating extensions!", "Setup Error");
    }
    
    
    for (u64 i = 0; i < instanceExtensions.size(); i++) {
        if (!ExtensionAvail(availableExtensions, instanceExtensions[i])) {
            pform.FatalError("Issue while checking for extensions", "Setup Error");
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
        static_cast<u32> (instanceExtensions.size()),
        instanceExtensions.data
    };
    
    if (validationLayers) {
        instInfo.enabledLayerCount = static_cast<u32>(debugLayers.size());
        instInfo.ppEnabledLayerNames = debugLayers.data;
    }
    
    if (vkCreateInstance(&instInfo, nullptr, &instance)  != VK_SUCCESS) {
        pform.FatalError("Issue while creating an instance", "Setup Error");
    }
    bool res = LoadInstanceFns();
    
    if (!res) {
        pform.FatalError("Issue while creating an instance", "Setup Error");
    }
    
    // Instance created, can ask for the presentation surface
#ifdef _WIN32
    
    VkWin32SurfaceCreateInfoKHR surf_info = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, 
        pform.window.inst, pform.window.handle
    };
    
    
    ENSURE_SUCC(vkCreateWin32SurfaceKHR(instance, &surf_info, nullptr, &surf));
#endif
    
    ENSURE_SUCC( vkEnumeratePhysicalDevices(instance, &numDevices, nullptr));
    Vector<VkPhysicalDevice> physDevs(numDevices);
    ENSURE_SUCC (vkEnumeratePhysicalDevices(instance, &numDevices, &physDevs[0]));
    // Loop over with all of the devices to see if an of them are good
    bool found = 0;
    u32 graphicsQueueFamilyIndex = 0xFFFFFFFF;
    
    for (u32 i  = 0; i < numDevices; i++) {
        // Check if the device is OK
        u32 numPhysExtensions;
        auto& dev = physDevs[i];
        if (vkEnumerateDeviceExtensionProperties (dev, nullptr, &numPhysExtensions, nullptr) != VK_SUCCESS) continue;
        Vector<VkExtensionProperties> availExtensions(numPhysExtensions);
        if (vkEnumerateDeviceExtensionProperties (dev, nullptr, &numPhysExtensions, &availExtensions[0]) != VK_SUCCESS) continue;
        
        for (u32 j = 0; j < deviceExtensions.size(); j++) {
            if ( !ExtensionAvail(  availExtensions, deviceExtensions[j] ) ) {
                continue;
            }
        }
        
        // check for properies and features
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        
        vkGetPhysicalDeviceProperties(dev, &props);
        vkGetPhysicalDeviceFeatures(dev, &features);
        if (!CheckProperties(props))
            continue;
        if (!CheckFeatures(features)) {
            continue;
        }
        
        u32 qFamilies;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &qFamilies, nullptr);
        if (qFamilies == 0) {
            continue;
        }
        
        Vector<VkQueueFamilyProperties> q_family_props(qFamilies);
        Vector<VkBool32> canPresent(qFamilies);
        
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &qFamilies, &q_family_props[0]);
        
        
        
        for (u32 j = 0; j < qFamilies; j++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(dev, j, surf, &canPresent[i]);
        }
        
        for (u32 j = 0; j < qFamilies; j++) {
            if ((q_family_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                if (canPresent[i] == VK_TRUE) {
                    graphicsQueueFamilyIndex = j;
                    gpu = dev;
                }
            }
        }
        
    }
    if (gpu == VK_NULL_HANDLE) {
        pform.FatalError("Unable to find a suitable physical device", "Vulkan Error");
    }
    gqFamilyIndex = graphicsQueueFamilyIndex;
    f32 qPriorities = 1.0f;
    
    VkDeviceQueueCreateInfo qCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr, 0, graphicsQueueFamilyIndex, 1, &qPriorities
    };
    
    VkDeviceCreateInfo devCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1,
        &qCreateInfo, 0, nullptr, (u32) deviceExtensions.size(),
        &deviceExtensions[0], nullptr
    };
    
    if (vkCreateDevice(gpu, &devCreateInfo, nullptr, &this->dev) != VK_SUCCESS) {
        pform.FatalError("Unable to create the device!", "Vulkan Setup Error");
    }
    
    if (!LoadDevFns()) {
        pform.FatalError((const char*)errString, "Vulkan Setup Error");  
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
        pform.FatalError("Could not check for surface presentation!", "Vulkan Swapchain Creation Error");
    }
    
    u32 formatsCount;
    ENSURE_SUCC( vkGetPhysicalDeviceSurfaceFormatsKHR (gpu, surf, &formatsCount, nullptr));
    if (formatsCount == 0) pform.FatalError("Unable to enumerate formats", "Vulkan Swapchain Creation Error");
    
    Vector<VkSurfaceFormatKHR> formats(formatsCount);
    ENSURE_SUCC( vkGetPhysicalDeviceSurfaceFormatsKHR (gpu, surf, &formatsCount, &formats[0]));
    
    u32 ct;
    ENSURE_SUCC ( vkGetPhysicalDeviceSurfacePresentModesKHR (gpu, surf, &ct, nullptr) );
    Vector<VkPresentModeKHR> presentModes(ct);
    ENSURE_SUCC ( vkGetPhysicalDeviceSurfacePresentModesKHR (gpu, surf, &ct, &presentModes[0]) );
    
    if (NUM_IMAGES > sCaps.maxImageCount || NUM_IMAGES < sCaps.minImageCount) {
        pform.FatalError("Could not get number of swapchain images", "Vulkan Swapchain Creation Error");
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
        wantedFormat = {VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
        if (formats[0].format == VK_FORMAT_UNDEFINED) wantedFormat = formats[0];
    }
    
    int w, h;
    pform.GetRect(&w, &h);
    ext = { clamp(sCaps.minImageExtent.width, sCaps.maxImageExtent.width, (u32)w ),
        clamp(sCaps.minImageExtent.height, sCaps.maxImageExtent.height, (u32)h)};
    
    VkImageUsageFlags flags =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (!(sCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        pform.FatalError("Unable to get proper flags", "Vulkan Swapchain Creation Error");
    }
    
    VkSurfaceTransformFlagBitsKHR transform = (sCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : sCaps.currentTransform;
    VkPresentModeKHR presMode = static_cast<VkPresentModeKHR>(-1);
    bool found = false, satisfied = false;
    for (u64 i = 0; i < presentModes.size(); i++ ) {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            presMode = presentModes[i];
            found = true;
        } else if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR && !found ) {
            presMode = presentModes[i];
            satisfied = true ;
        }
    }
    
    if (!satisfied && !found) {
        pform.FatalError("Not able to get a suitable mode without screen tearing", "Vulkan Setup Error");
    }
    
    
    auto oldHandle = sc.handle;
    
    VkSwapchainCreateInfoKHR scCreateInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surf,
        NUM_IMAGES,
        wantedFormat.format,
        wantedFormat.colorSpace,
        ext,
        1,
        flags,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        transform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presMode,
        VK_TRUE,
        oldHandle
    };
    
    
    sc.format = wantedFormat;
    
    ENSURE_SUCC ( vkCreateSwapchainKHR(dev, &scCreateInfo, nullptr, &sc.handle)   );
    
    
    if (oldHandle == VK_NULL_HANDLE) {
        // Destroy the old swapchain
        vkDestroySwapchainKHR(dev, oldHandle, nullptr);
    }
    
    u32 numactual;
    
    auto res_swap = vkGetSwapchainImagesKHR(dev, sc.handle, &numactual, nullptr);
    if ( res_swap != VK_SUCCESS || numactual != NUM_IMAGES) {
        pform.FatalError("Error with initialization of swapchain", "Swapchain creation Error!");
    }
    
    VkImage tmp_images[3];
    
    ENSURE_SUCC (vkGetSwapchainImagesKHR(dev, sc.handle, &numactual, &tmp_images[0] ));
    
    for (int i = 0; i < numactual; i++) {
        images[i].handle = tmp_images[i];
    }
    
    
    for (int i = 0; i < NUM_IMAGES; i++ ) {
        VkImageViewCreateInfo createInfo = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, images[i].handle,
            VK_IMAGE_VIEW_TYPE_2D, sc.format.format,
            {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY
            },
            // VkImageSubresourceRange
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            }
        };
        
        ENSURE_SUCC (vkCreateImageView(dev, &createInfo, nullptr, &images[i].view));
        
    }
    pform.window.renderable = true;
    
    
    
    
}



void VulkanContext::Semaphores(void) {
    VkSemaphoreCreateInfo semInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    };

    for (int i = 0; i < NUM_IMAGES; i++ ) {
        ENSURE_SUCC (vkCreateSemaphore(dev, &semInfo, nullptr, &frameResources[i].renderingReady));
        ENSURE_SUCC (vkCreateSemaphore(dev, &semInfo, nullptr, &frameResources[i].presentReady));
    }
    
}

void VulkanContext::Init(void) {
    pform.Init();
    LoadExportedFns();
    LoadGlobalFns();
    Device(true);
    Swapchain();
    vkGetDeviceQueue(dev, gqFamilyIndex, 0, &graphicsPresentQueue);
    Semaphores();    
    AllocCmdBuffers();
    
}

void VulkanContext::CreateCmdPool(VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        flags,
        gqFamilyIndex
    };

    if (vkCreateCommandPool(dev, &createInfo, nullptr, &cmdPool) != VK_SUCCESS) {
        pform.FatalError("Unable to create command pools", "Vulkan Command Pool Error");
    }
}

// (TODO) maybe some sort of advanced args or something
void VulkanContext::AllocCmdBuffers(void) {
    CreateCmdPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

    for (u64 i = 0; i <  NUM_IMAGES; i++ ) {
        VkCommandBufferAllocateInfo cbAllocInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            nullptr,
            cmdPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1
        };
        ENSURE_SUCC ( vkAllocateCommandBuffers(dev, &cbAllocInfo, &frameResources[i].commandBuffer));
        
    }

    
    
}
