// Does the device setup necessary for Vulkan
#include "vulkan.h"

#define VK_EXPORTED_FUNCTION( fun ) PFN_##fun fun;
#define VK_GLOBAL_LEVEL_FUNCTION( fun ) PFN_##fun fun;
#define VK_INSTANCE_LEVEL_FUNCTION( fun ) PFN_##fun fun;
#define VK_DEVICE_LEVEL_FUNCTION( fun ) PFN_##fun fun;
#include "functionslist.h"



#include "device.h"
#include "platform.h"

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


void EnsureSuperset(std::vector<const char*>& given, std::vector<const char*>& wanted) {

	for (size_t i = 0; i < wanted.size(); i++) {
		bool found = false;
		for (size_t j = 0; j < given.size(); j++) {
			const char* wantedStr = wanted[i];
			const char* givenStr = given[i];
			if (!strcmp(wantedStr, givenStr)) {
				found = true;
				break;
			} 
		}
		if (!found) {
			FatalError(wanted[i], "Unable to find given extension!");
		}
	}
}

void CheckValLayers(void) {
	
}

void VulkanContext::Init(bool validationLayers) {
	if (validationLayers) {
		checkValidationLayersSupport();
	}
}
