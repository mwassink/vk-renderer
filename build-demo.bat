setlocal
if not exist "build/" mkdir build
cd build
clang++ -D _CRT_SECURE_NO_WARNINGS -maes -msse4 -g ..\device.cpp ..\platform_win32.cpp ..\renderer.cpp ..\utils.cpp  ..\tests\basicmodel.cpp -luser32 -lgdi32 -o debug.exe -I C:\VulkanSDK\1.2.198.1\Include\vulkan -D VK_NO_PROTOTYPES -D VK_USE_PLATFORM_WIN32_KHR -D USE_SWAPCHAIN_EXTENSIONS
endlocal 
