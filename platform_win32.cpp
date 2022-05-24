// platform.cpp

// Neew windows to implement some of these
#include <windows.h>
#include "platform.h"

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
    wcex.lpszClassName = "Mark Vulkan";
    wcex.hIconSm = NULL;
    
    if( !RegisterClassEx( &wcex ) ) {
        return false;
    }
    
    // Create window
    Handle = CreateWindow( "Mark Vulkan", title, WS_OVERLAPPEDWINDOW, 20, 20, 500, 500, nullptr, nullptr, Instance, nullptr );
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



