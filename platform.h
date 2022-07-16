#ifndef PLATFORM_H
#define PLATFORM_H

extern const char* swapChainExtensionName;
extern const char* surfaceExtensionName;
extern const char* platformSurfaceExtensionName;


#ifdef _WIN32
#include <windows.h>
#include "types.h"
struct Window {

    HINSTANCE inst =0;
    HWND handle = 0;
    bool swapchainValid = false;
    bool finished = false;
    int w, h;
   	bool Create(const char*);
   	void Show();
   	void Update();
    void GetRect(int* w, int* h);


};


typedef HMODULE ModuleHandle ;

#else

struct Window {
	// some Lunix stuff here?
};


#endif

struct FileData {
    unsigned char* data;
    unsigned long size;
};

struct Platform{
	Window window;
	ModuleHandle vkLib;
	int pageSize;
	bool Init();
	void GetRect(int* w, int* h);
	void* Wrangle(const char* functionName);
	void FatalError(const char*, const char* );
	void PopupWarning(const char* msg, const char* title);
	void* WrangleExportedEntry(const char* functionName);

  FileData ReadBinaryFile(const char* name);
  void ReleaseFileData(FileData* data);
	void* GetMemory(u32 count, u32* bytesReturned);
	void FreeMemory(void* data);
	void ShowWindow(void);
    
};

#endif //PLATFORM_H
