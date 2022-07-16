// platform.cpp

// Neew windows to implement some of these
#include <windows.h>
#include "platform.h"
#include "vulkan.h"

const char* swapChainExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
const char* surfaceExtensionName = VK_KHR_SURFACE_EXTENSION_NAME;
const char* platformSurfaceExtensionName= VK_KHR_WIN32_SURFACE_EXTENSION_NAME;

extern Platform* pform;

int __WINDOWHEIGHT = 0;
int __WINDOWWIDTH = 0;
bool __SWAPCHAINFLAG = 0;
bool __FINISHED = 0;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SIZE:
            RECT rect;
            GetClientRect(hWnd, &rect);
            __WINDOWWIDTH = rect.right - rect.left;
            __WINDOWHEIGHT = rect.bottom - rect.top;
            
            __SWAPCHAINFLAG = true;


        break;
        case WM_KEYDOWN:
        case WM_CLOSE:
            __FINISHED = true;
        break;
        default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        
    }
    return 0;
}

void Window::Show(void) {
    ShowWindow(handle, SW_SHOWNORMAL);
    UpdateWindow(handle);
}


void Window::Update(void) {

    MSG msg = {};

    if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);   
    }

    w = __WINDOWWIDTH;
    h = __WINDOWHEIGHT;
    swapchainValid = !__SWAPCHAINFLAG;
    __SWAPCHAINFLAG = false;
    finished = __FINISHED;
    

}


bool Window::Create(const char* title) {
    inst = GetModuleHandle(nullptr);
    WNDCLASS  WindowClass = {};
    WindowClass.lpfnWndProc = WndProc;
    WindowClass.hInstance = inst;
    WindowClass.lpszClassName = "Renderer";
    
    if( !RegisterClass( &WindowClass ) ) {
        return false;
    }
    
    // Create window
    handle = CreateWindowExA(0, "Renderer", "Renderer Test", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, inst, 0);
    w = 500;
    h = 500;
    if( !handle ) {
        return false;
    }
    
    return true;
    
}

void Platform::GetRect(int* w, int* h) {
    *w = window.w;
    *h = window.h;
}

void Window::GetRect(int* w, int* h) {
    RECT rect;
    GetWindowRect(handle, &rect);
    *w = rect.right - rect.left;
    *h = rect.bottom - rect.top;
}

bool Platform::Init() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    pageSize = sysInfo.dwAllocationGranularity;
    if (!window.Create("Vulkan Test")) {
        return false;
    }
	vkLib = LoadLibrary("vulkan-1.dll");
	if (!vkLib) {
		FatalError("Unable to init vulkan library", "Init Error");
		return false;
	}
    pform = this;
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

void* Platform::GetMemory(u32 ct, u32* bytesReturned) {
    if (ct == 0) return 0;
    void* ptr = VirtualAlloc(NULL, ct, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    *bytesReturned = ptr ? ((ct - 1) / pageSize + 1) * pageSize : 0;
    return ptr;
}

void Platform::FreeMemory(void* data) {
    VirtualFree(data, 0, MEM_RELEASE);
}

void Platform::ShowWindow() {
    window.Show();
}

