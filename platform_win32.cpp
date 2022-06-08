// platform.cpp

// Neew windows to implement some of these
#include <windows.h>
#include "platform.h"
#include "vulkan.h"

const char* swapChainExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
const char* surfaceExtensionName = VK_KHR_SURFACE_EXTENSION_NAME;
const char* platformSurfaceExtensionName= VK_KHR_WIN32_SURFACE_EXTENSION_NAME;




LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SIZE:
        case WM_EXITSIZEMOVE:
        PostMessage(hWnd, WM_USER + 1, wParam, lParam);
        break;
        case WM_KEYDOWN:
        case WM_CLOSE:
        PostMessage(hWnd, WM_USER + 2, wParam, lParam);
        break;
        default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        
    }
    return 0;
}





bool Window::Create(const char* title) {
    inst = GetModuleHandle(nullptr);
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = inst;
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
    handle = CreateWindow( "Vulkan Renderer", title, WS_OVERLAPPEDWINDOW, 20, 20, 500, 500, nullptr, nullptr, inst, nullptr );
    if( !handle ) {
        return false;
    }
    
    return true;
    
}

void Platform::GetRect(int* w, int* h) {
    window.GetRect(w, h);
}

void Window::GetRect(int* w, int* h) {
    RECT rect;
    GetWindowRect(handle, &rect);
    *w = rect.right - rect.left;
    *h = rect.bottom - rect.top;
}

bool Platform::Init() {
    if (!window.Create("Vulkan Test")) {
        return false;
    }
	vkLib = LoadLibrary("vulkan-1.dll");
	if (!vkLib) {
		FatalError("Unable to init vulkan library", "Init Error");
		return false;
	}
	return true;
}


void* Platform::WrangleExportedEntry(const char* functionName) {
	void* target = (void*)GetProcAddress(vkLib, functionName);
    return target;
    
}




void Platform::FatalError(const char* msg, const char* title) {
    MessageBox(0, msg, title, MB_OK | MB_ICONERROR);
    ExitProcess(1);
}



void Platform::PopupWarning(const char* msg, const char* title) {
    MessageBox(0, msg, title, MB_OK | MB_ICONWARNING);
}

FileData Platform::ReadBinaryFile(const char* name) {
    FileData fileData;
    HANDLE hFile;
    union _LARGE_INTEGER size;
    DWORD  read;
    hFile = CreateFileA(name, GENERIC_READ, 0, 0, OPEN_EXISTING,0 , 0 );
    if (!GetFileSizeEx(hFile, &size )) {
        FatalError(name, "Unable to open file!");
    }
    fileData.data = (unsigned char*)VirtualAlloc(NULL, size.QuadPart + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
    if (!ReadFile(hFile, fileData.data, size.QuadPart, &read, 0)) {
        FatalError(name, "Unable to open file");
    }
    fileData.size = read;
    CloseHandle(hFile);
    return fileData;
}

void Platform::ReleaseFileData(FileData* data) {
    VirtualFree(data->data, 0, MEM_RELEASE);
}

