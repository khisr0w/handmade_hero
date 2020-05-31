#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

// TODO Implement Sine ourselves
#include <math.h>

#define local_persist static 
#define global_var static
#define internal static
#define PI32 3.14159265359f

typedef int32_t bool32;
typedef char bool8;
typedef float real32;
typedef double real64;

struct win32_offscreen_buffer {

    BITMAPINFO Info;
    void *Memory;
    int Width; 
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct win32_window_dimension {
    int Width;
    int Height;
};

// TODO have to change this from global, initialized to zero by default
global_var bool8 GlobalRunning;
global_var win32_offscreen_buffer GlobalBackBuffer; 
global_var LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

// XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
	
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_var x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
	
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_var x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// DirectSound
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void Win32LoadXInput(void) {

    // TODO Diagnostic
    HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
    if(!XInputLibrary) {

	HMODULE XInputLibrary = LoadLibrary("xinput9_1_0.dll");
    }

    if(!XInputLibrary) {

	HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
    }

    if(XInputLibrary) {

	XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");  
	XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");  
    } else {
	// TODO Diagnostic
    }
}

internal void Win32InitDSound(HWND Window, int32_t SamplesPerSecond, int32_t BufferSize) {

    // Load Library
    HMODULE DSoundLibrary = LoadLibrary("dsound.dll");
    if(DSoundLibrary) {

	// Get a DirectSound object
    	direct_sound_create *DirectSoundCreate = 
	    (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

	// TODO Must check if it works with XP, is it DirectSound 8 or 7?
	LPDIRECTSOUND DirectSound;
	if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {

	    WAVEFORMATEX WaveFormat = {};
	    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	    WaveFormat.nChannels = 2;
	    WaveFormat.wBitsPerSample = 16;
	    WaveFormat.nSamplesPerSec = SamplesPerSecond;
	    WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8 ;
	    WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
	    WaveFormat.cbSize = 0;

	    if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {

		// Create a Primary Buffer
		DSBUFFERDESC BufferDescription = {};
		BufferDescription.dwSize = sizeof(BufferDescription);
		BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

		LPDIRECTSOUNDBUFFER PrimaryBuffer;
		if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))) {
		    
		    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))) {
			// Format is set
			OutputDebugString("Primary Buffer Set!\n");
		    } else {
			// TODO Diagnostic
		    }

		} else {

		    // TODO Diagnostic
		}
		
	    } else {
		
		// TODO Diagnostic
	    }

	    // Create a Secondary Buffer
	    DSBUFFERDESC BufferDescription = {};
	    BufferDescription.dwSize = sizeof(BufferDescription);
	    BufferDescription.dwFlags = 0;
	    BufferDescription.dwBufferBytes = BufferSize;
	    BufferDescription.lpwfxFormat = &WaveFormat;
	    if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0))) {

		// Start it playing
		OutputDebugString("Secondary Buffer Set!\n");

	    } else {

		// TODO Diagnostic
	    }
	} else {
	    
	    // TODO Diagnostic
	}

    } else {
	// TODO Diagnostic
    }
}

internal win32_window_dimension Win32GetWindowDimension(HWND Window) {
    
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Height = ClientRect.bottom - ClientRect.top;
    Result.Width = ClientRect.right - ClientRect.left;

    return (Result);
}

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset) {
    
    // TODO let's what the optimizer does

    uint8_t *Row = (uint8_t *)Buffer->Memory;
    for(int Y = 0; Y < Buffer->Height; ++Y) {

	uint32_t *Pixel = (uint32_t *)Row;
	for(int X = 0; X < Buffer->Width; ++X) {

	    uint8_t Blue = (X + XOffset);
	    uint8_t Green = (Y + YOffset);
	    // Coloring scheme is BRG because fuck windows
	    *Pixel++ = ((Green << 8) | Blue);
	}
	Row += Buffer->Pitch;
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
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    // TODO probably clear this to black

    Buffer->Pitch = Width*Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext,
					int WindowWidth, int WindowHeight) {
    // TODO Aspect ration correction
    StretchDIBits(
	    DeviceContext,
	    /*
	    X, Y, Width, Height,
	    X, Y, Width, Height,
	    */
	    0, 0, WindowWidth, WindowHeight,
	    0, 0, Buffer->Width, Buffer->Height,
	    Buffer->Memory,
	    &Buffer->Info,
	    DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT CALLBACK Win32MainWindowCallback(
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
	    // TODO Handle this, this might be an error. 
	    GlobalRunning = 0;

        } break;

        case WM_CLOSE:
        {
	    // TODO Handle this
	    GlobalRunning = 0;

        } break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
	    uint32_t VKCode = wParam;
	    bool32 WasDown = (lParam & (1 << 30)); 
	    bool32 IsDown = (lParam & (1 << 31));

	    if (WasDown != IsDown) {

		if(VKCode == 'W') {
		}
		else if(VKCode == 'A') {
		}
		else if(VKCode == 'S') {
		}
		else if(VKCode == 'D') {
		}
		else if(VKCode == 'E') {
		}
		else if(VKCode == 'Q') {
		}
		else if(VKCode == VK_UP) {
		}
		else if(VKCode == VK_DOWN) {
		}
		else if(VKCode == VK_RIGHT) {
		}
		else if(VKCode == VK_LEFT) {
		}
		else if(VKCode == VK_ESCAPE) {
		    OutputDebugString("Escape: ");
		    if(IsDown) {
			OutputDebugString("IsDown ");
		    }
		    if(WasDown) {
			OutputDebugString("WasDown ");
		    }
		    OutputDebugString("\n ");
		}
		else if(VKCode == VK_SPACE) {
		}

		bool32 AltKeyWasDown = (lParam & (1 << 29));
		if((VKCode == VK_F4) && AltKeyWasDown) {

		    GlobalRunning = 0;
		}
	    }

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
	    Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);

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

struct win32_sound_output {
    
    int SamplesPerSecond; 
    int ToneHz; 
    int16_t ToneVolume;
    uint32_t RunningSampleIndex;
    int WavePeriod;
    int BytesPerSample;
    int SecondaryBufferSize;
    real32 tSine;
    int LatencySampleCount;
};

internal void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite) {
    
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, 
						&Region1, &Region1Size,
						&Region2, &Region2Size,
						     0))) {

        // TODO assert that RegionSize1/RegionSize2 is valid
        int16_t *SampleOut = (int16_t *)Region1;
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)  {

            real32 SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * SoundOutput->ToneVolume); 
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;

	    SoundOutput->tSine += 2.0f*PI32 * 1.0f / (real32)SoundOutput->WavePeriod;
            ++SoundOutput->RunningSampleIndex;
        }

        SampleOut = (int16_t *)Region2;
        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)  {

            real32 SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * SoundOutput->ToneVolume); 
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;

	    SoundOutput->tSine += 2.0f*PI32 * 1.0f / (real32)SoundOutput->WavePeriod;
            ++SoundOutput->RunningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);

    }   
}
int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode) {
    
    Win32LoadXInput();

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

	    // Input Variables
	    int XOffset = 0;
	    int YOffset = 0;

	    // Sound Variables
	    win32_sound_output SoundOutput = {};
	    SoundOutput.SamplesPerSecond = 48000;
	    SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
	    SoundOutput.ToneHz = 256;
	    SoundOutput.ToneVolume = 6000;
	    SoundOutput.RunningSampleIndex = 0;
	    SoundOutput.WavePeriod =SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
	    SoundOutput.BytesPerSample = sizeof(int16_t)*2;
	    SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
    
	    Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
	    Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample);
	    GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

	    GlobalRunning = 1;
            while(GlobalRunning) {

		MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
		    
		    if(Message.message == WM_QUIT) {
			GlobalRunning = 0;
		    }

		    TranslateMessage(&Message);
		    DispatchMessage(&Message);
   
		}
		// TODO should we pull this more frequently
		for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex) {

		    XINPUT_STATE ControllerState;
		    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
			
			//Controller Connected
			// TODO See the behavior of dwPacketNumber
			XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;	

			bool32 Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
			bool32 Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
			bool32 Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
			bool32 Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
			bool32 Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
			bool32 Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
			bool32 LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			bool32 RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
			bool32 AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
			bool32 BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
			bool32 XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
			bool32 YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

			int16_t StickX = Pad->sThumbLX;
			int16_t StickY = Pad->sThumbLY;

			// TODO handle deadzone for the thumb sticks
			XOffset += StickX / 4096;
			YOffset += StickY / 4096;

			SoundOutput.ToneHz = 512 + (int)(256.0f * (real32)StickY / 30000.0f);
			SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;

		    } else {

			// Controller Not Available
		    }
		}

		XINPUT_VIBRATION Vibration;
		Vibration.wLeftMotorSpeed = 60000;
		Vibration.wRightMotorSpeed = 60000;
		// XInputSetState(0, &Vibration);

		RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

		// DirectSound Output test
		DWORD PlayCursor;
		DWORD WriteCursor;
		if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))) {

		    DWORD ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) % 
					SoundOutput.SecondaryBufferSize);

		    DWORD TargetCursor = (PlayCursor +
			(SoundOutput.LatencySampleCount*SoundOutput.BytesPerSample)) % 
					SoundOutput.SecondaryBufferSize;

		    DWORD BytesToWrite;

		    // TODO change this to a lower latency offset from the playcursor when we have FX
		    if (ByteToLock > TargetCursor) {
			BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
			BytesToWrite += TargetCursor;
		    } else {
			BytesToWrite = TargetCursor - ByteToLock;
		    }

		    // TODO Test Required
		    
		    Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite); 
		}
		
		win32_window_dimension Dimension = Win32GetWindowDimension(Window);
		HDC DeviceContext = GetDC(Window);
		Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, 
			Dimension.Width, Dimension.Height);
		ReleaseDC(Window, DeviceContext);
	    }

        } else {

        }

    } else {

    }

    return 0;
}
