/* date = May 22nd 2022 2:39 pm */

#ifndef PLATFORM_H
#define PLATFORM_H


#ifdef WIN32
struct Window {
    
    HINSTANCE Instance =0;
    HWND Handle = 0;
    bool Renderable = false;
    
   	bool Create(void);
   	void Show();
   	void Update();


};


typedef ModuleHandle HMODULE;

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

}

#endif //PLATFORM_H
