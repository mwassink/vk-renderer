/* date = May 22nd 2022 2:39 pm */

#ifndef PLATFORM_H
#define PLATFORM_H

extern const char* swapChainExtensionName;
extern const char* surfaceExtensionName;
extern const char* platformSurfaceExtensionName;


#ifdef WIN32
struct Window {
    
    HINSTANCE inst =0;
    HWND handle = 0;
    bool renderable = false;
    
   	bool Create(void);
   	void Show();
   	void Update();


};


typedef HMODULE ModuleHandle ;

#else

struct Window {
	// some Lunix stuff here?
}


#endif


struct Platform{
	Window window;
	ModuleHandle vkLib;
	bool Init();
	void GetRect(int* w, int* h);
	void* Wrangle(const char* functionName);
	void FatalError(void);
	void PopupWarning(const char* msg, const char* title);
	


};

#endif //PLATFORM_H
