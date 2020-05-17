#include <windows.h>

LRESULT CALLBACK MainWindowCallback(
    HWND Window, 
    UINT Message,
    WPARAM wParam,
    LPARAM lParam) {

    LRESULT Result = 0;

    switch (Message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");

        } break;

        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");

        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;
            static DWORD Operation = WHITENESS;
            PatBlt(DeviceContext, X, Y, Width, Height, Operation);
            if (Operation == WHITENESS) {
                Operation = BLACKNESS;
            } else {
                Operation = WHITENESS;
            }
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
    WindowClass.lpfnWndProc= MainWindowCallback;
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

            MSG Message;
            for(;;) {

                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if (MessageResult>0) {

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                } else {
                    break;
                }
            }

        } else {

        }

    } else {

    }

    return 0;
}