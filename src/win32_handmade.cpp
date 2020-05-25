#include <windows.h>
#include <stdint.h>

#define local_persist static 
#define global_var static
#define internal static

struct win32_offscreen_buffer {

    BITMAPINFO Info;
    void *Memory;
    int Width; 
    int Height;
    int Pitch;
    int BytesPerPixel;
};

// TODO have to change this from global, initialized to zero by default
global_var bool GlobalRunning;
global_var win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension {
    int Width;
    int Height;
};

win32_window_dimension Win32GetWindowDimension(HWND Window) {
    
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Height = ClientRect.bottom - ClientRect.top;
    Result.Width = ClientRect.right - ClientRect.left;

    return (Result);
}

internal void RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset) {
    
    // TODO let's what the optimizer does

    uint8_t *Row = (uint8_t *)Buffer.Memory;
    for (int Y = 0; Y < Buffer.Height; ++Y) {

	uint32_t *Pixel = (uint32_t *)Row;
	for(int X = 0; X < Buffer.Width; ++X) {

	    uint8_t Blue = (X + XOffset);
	    uint8_t Green = (Y + YOffset);
	    // Coloring scheme is BRG because fuck windows
	    *Pixel++ = ((Green << 8) | Blue);
	}
	Row += Buffer.Pitch;
    }
}
internal void Win32ResizeDIBSection (win32_offscreen_buffer *Buffer, int Width, int Height) {

    // TODO we have to catch the edge cases
    // Don't have to free the memory first, free after, then free first if that fails

    if(Buffer->Memory) {
	VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
   
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width; 
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // for top-down DIB
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    // Allocate memory for the Bitmap
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // TODO probably clear this to black

    Buffer->Pitch = Width*Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferInWindow(HDC DeviceContext,
					int WindowWidth, int WindowHeight,
					win32_offscreen_buffer Buffer) {
    //TODO Aspect ration correction
    StretchDIBits(
	    DeviceContext,
	    /*
	    X, Y, Width, Height,
	    X, Y, Width, Height,
	    */
	    0, 0, WindowWidth, WindowHeight,
	    0, 0, Buffer.Width, Buffer.Height,
	    Buffer.Memory,
	    &Buffer.Info,
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
        } break;

        case WM_DESTROY:
        {
	    //TODO Handle this, this might be an error. 
	    GlobalRunning = false;
            OutputDebugStringA("WM_DESTROY\n");

        } break;

        case WM_CLOSE:
        {
	    //TODO Handle this
	    GlobalRunning = false;
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

	    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
	    Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer);

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

    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
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

	    GlobalRunning = true;
            while(GlobalRunning) {

		MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
		    
		    if(Message.message == WM_QUIT) {
			GlobalRunning = false;
		    }

		    TranslateMessage(&Message);
		    DispatchMessage(&Message);
   
		}

		RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

		HDC DeviceContext = GetDC(Window);
		RECT ClientRect;
		GetClientRect(Window, &ClientRect);
		int WindowHeight = ClientRect.bottom - ClientRect.top;
		int WindowWidth = ClientRect.right - ClientRect.left;

		win32_window_dimension Dimension = Win32GetWindowDimension(Window);
		Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer);
		ReleaseDC(Window, DeviceContext);

		++XOffset;
		++YOffset;

	    }

        } else {

        }

    } else {

    }

    return 0;
}

