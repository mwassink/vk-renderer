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


void VulkanContext::Init(bool validationLayers) {
	if (debugLayers) {
		CheckDebugLayers();
	}

	u32 numAvail, numDevices;
	if (vkEnumerateInstanceExtensionProperties(nullptr, &numAvail, nullptr) != VK_SUCCESS) {
		FatalError("Issue when finding the number of extensions");
	}
	std::vector<const char*> instanceExtensions  = { platformSurfaceExtensionName, surfaceExtensionName, swapChainExtensionName}; // one of these doesn't belong
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

    ENSURE_SUCC( vkEnumeratePhysicalDevices(instance, numDevices, nullptr));
    std::vector<VkPhysicalDevice> physDevs(numDevices);
    ENSURE_SUCC (vkEnumeratePhysicalDevices(instance, &numDevices, &physDevs));
    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    
    

    

    
    

    
}
