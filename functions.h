
#include "vulkan.h"

#define VK_E_FN( fun ) extern PFN_##fun fun;
#define VK_G_FN( fun ) extern PFN_##fun fun;
#define VK_IN_FN( fun ) extern PFN_##fun fun;
#define VK_D_FN( fun ) extern PFN_##fun fun;

#include "functionslist.h"


