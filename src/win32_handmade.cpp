#include <windows.h>

#define local_persist static 
#define global_var static
#define internal static


// TODO have to change this from global, initialized to zero by default
global_var bool Running;

global_var BITMAPINFO BitmapInfo;
global_var void *BitmapMemory;
global_var HBITMAP BitmapHandle; 
global_var HDC BitmapDeviceContext; 

internal void Win32ResizeDIBSection (int Width, int Height) {

    // TODO we have to catch the edge cases
    // Don't have to free the memory first, free after, then free first if that fails
    
    if(BitmapHandle) {

	DeleteObject(BitmapHandle);
    }

    if(!BitmapDeviceContext) {

	// TODO should we recreate these under certain circumstances
	HDC BitmapDeviceContext = CreateCompatibleDC(0);
    }
    
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    
    BitmapHandle = CreateDIBSection(
	    BitmapDeviceContext, 
	    &BitmapInfo,
	    DIB_RGB_COLORS,
	    &BitmapMemory,
	    0, 0);	
}

internal void Win32UpdateWindow (HDC DeviceContext, int X, int Y, int Width, int Height) {
    
    StretchDIBits(
	    DeviceContext,
	    X, Y, Width, Height,
	    X, Y, Width, Height,
	    BitmapMemory,
	    &BitmapInfo,
	    DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(
    HWND Window, 
    UINT Message,
    WPARAM wParam,
    LPARAM lParam) {

    LRESULT Result = 0;

    switch (Message)
    {
        case WM_SIZE:
        {
	    RECT ClientRect;
	    GetClientRect(Window, &ClientRect);
	    int Height = ClientRect.bottom - ClientRect.top;
	    int Width = ClientRect.right - ClientRect.left;
	    Win32ResizeDIBSection(Width, Height);

        } break;

        case WM_DESTROY:
        {
	    //TODO Handle this, this might be an error. 
	    Running = false;
            OutputDebugStringA("WM_DESTROY\n");

        } break;

        case WM_CLOSE:
        {
	    //TODO Handle this
	    Running = false;
            OutputDebugStringA("WM_CLOSE\n");

        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
	    Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
            EndPaint(Window, &Paint);

        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");

        } break;

        default:
        {
            // OutputDebugStringA("WM_DEFAULT\n");
            Result = DefWindowProc(Window, Message, wParam, lParam);
        } break;
    }

    return(Result);
}


int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode) {
    
    // Making a window class
    WNDCLASS WindowClass = {};

    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc= Win32MainWindowCallback;
    WindowClass.hInstance= Instance;
    WindowClass.lpszClassName=TEXT("HandmadeHeroWindowClass");
    // WindowClass.hIcon
    // LPCSTR    lpszClassName;

    // Registering a Window Class
    if (RegisterClass(&WindowClass)) {

        HWND WindowHandle = CreateWindowEx(
            0,
            WindowClass.lpszClassName,
            TEXT("Handmade Hero"),
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0
        );

        if (WindowHandle) {

	    Running = true;
            MSG Message;
            while(Running) {

                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);

		TranslateMessage(&Message);
		DispatchMessage(&Message);
            }

        } else {

        }

    } else {

    }

    return 0;
}

