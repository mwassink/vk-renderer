setlocal
if not exist "build/" mkdir build
cd build
call cl -FC -Zi ..\main.cpp device.cpp platform.cpp user32.lib /I %VULKAN_SDK%\Include\vulkan -D VK_NO_PROTOTYPES -D VK_USE_PLATFORM_WIN32_KHR -D USE_SWAPCHAIN_EXTENSIONS /EHsc
endlocal 
