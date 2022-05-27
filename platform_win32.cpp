// platform.cpp

// Neew windows to implement some of these
#include <windows.h>
#include "platform.h"
#include "vulkan.h"

const_char* swapChainExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME ;
const_char* surfaceExtensionName = VK_KHR_SURFACE_EXTENSION_NAME ;
const_char* platformSurfaceExtensionName= VK_KHR_WIN32_SURFACE_EXTENSION_NAME;


bool Window::Create(const char* title) {
    Instance = GetModuleHandle(nullptr);
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = Instance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "Vulkan Renderer";
    wcex.hIconSm = NULL;
    
    if( !RegisterClassEx( &wcex ) ) {
        return false;
    }
    
    // Create window
    Handle = CreateWindow( "Vulkan Renderer", title, WS_OVERLAPPEDWINDOW, 20, 20, 500, 500, nullptr, nullptr, Instance, nullptr );
    if( !Handle ) {
        return false;
    }
    
    return true;
    
}

void Window::GetRect(int* w, int* h) {
		RECT rect;
		GetWindowRect(window.Handle, &rect);
		*w = ext.width;
		*h = ext.height;
}

bool Platform::Init() {
	vkLib = LoadLibrary("vulkan-1.dll");
	if (!vkLib) {
		printf("Unable to init vulkan library");
		return false;
	}
	return true;
}


void Platform::WrangleExportedEntry(const char* functionName, void* target) {
	target = LoadProcAddress(vkLib, functionName);
	return (target != nullptr);

}


void Platform::FatalError(const char* msg, const char* title) {
    MessageBox(0, msg, title, MB_OK | MB_ICONERROR);
    ExitProcess(1);
}

void Platform::PopupWarning(const char* msg, const char* title) {
    MessageBox(0, msg, title, MB_OK | MB_ICONWARNING);
}


