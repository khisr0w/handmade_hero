#include <windows.h>
#include <stdint.h>

#define local_persist static 
#define global_var static
#define internal static


// TODO have to change this from global, initialized to zero by default
global_var bool Running;

global_var BITMAPINFO BitmapInfo;
global_var void *BitmapMemory;
global_var int BitmapWidth; 
global_var int BitmapHeight;
global_var int BytesPerPixel = 4;

internal void RenderWeirdGradient(int XOffset, int YOffset) {
    
    int Width = BitmapWidth;
    int Height = BitmapHeight;

    int Pitch = Width*BytesPerPixel;
    uint8_t *Row = (uint8_t *)BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; ++Y) {

	uint32_t *Pixel = (uint32_t *)Row;
	for(int X = 0; X < BitmapWidth; ++X) {

	    uint8_t Blue = (X + XOffset);
	    uint8_t Green = (Y + YOffset);
	    // Coloring scheme is BRG because fuck windows
	    *Pixel++ = ((Green << 8) | Blue);
	}
	Row += Pitch;
    }
}
internal void Win32ResizeDIBSection (int Width, int Height) {

    // TODO we have to catch the edge cases
    // Don't have to free the memory first, free after, then free first if that fails

    if(BitmapMemory) {
	VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }
   
    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth; 
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight; // for top-down DIB
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    // Allocate memory for the Bitmap
    int BitmapMemorySize = (Width*Height)*BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // TODO probably clear this to black
}

internal void Win32UpdateWindow (HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height) {
    
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;
    
    StretchDIBits(
	    DeviceContext,
	    /*
	    X, Y, Width, Height,
	    X, Y, Width, Height,
	    */
	    0, 0, BitmapWidth, BitmapHeight,
	    0, 0, WindowWidth, WindowHeight,
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

	    RECT ClientRect;
	    GetClientRect(Window, &ClientRect);

	    Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
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

        HWND Window = CreateWindowEx(
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

        if (Window) {

	    int XOffset = 0;
	    int YOffset = 0;

	    Running = true;
            while(Running) {

		MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
		    
		    if(Message.message == WM_QUIT) {
			Running = false;
		    }

		    TranslateMessage(&Message);
		    DispatchMessage(&Message);
   
		}

		RenderWeirdGradient(XOffset, YOffset);

		HDC DeviceContext = GetDC(Window);
		RECT ClientRect;
		GetClientRect(Window, &ClientRect);
		int WindowHeight = ClientRect.bottom - ClientRect.top;
		int WindowWidth = ClientRect.right - ClientRect.left;
		Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
		ReleaseDC(Window, DeviceContext);

		++XOffset;

	    }

        } else {

        }

    } else {

    }

    return 0;
}

