/*
   TODO THIS IS NOT A FINAL PLATFORM LAYER

   - Saved Game Location
   - Getting a Handle to our own executable file
   - Asset loading Path
   - Threading (lauch a thread)
   - Raw Input (Support for multiple keyboards)
   - Sleep/timeBeginPeriod
   - ClipCursor() for multi monitor 
   - Fullscreen support
   - WM_SETCUROSOR (control cursor visibility)
   - QueryCancelAutoplay
   - WM_ACTIVEAPP (for when we are not the active application)
   - Blit Speed Improvement
   - Hardware Acceleration (OpenGL or DirectX or BOTH?)
   - GetKeyboardLayout (for the French Keyboard, international WASD keyboard)

   NOT A COMPLETE LIST!!!
*/

// TODO Implement Sine ourselves
#include <math.h>
#include <stdint.h>

#define local_persist static 
#define global_var static
#define internal static

#define PI32 3.14159265359f

typedef int32_t bool32;
typedef char bool8;
typedef float real32;
typedef double real64;

#include "handmade.cpp"

#include <windows.h>
#include <xinput.h>
#include <dsound.h>

#include "win32_handmade.h"

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
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename) {

	debug_read_file_result Result = {};

    HANDLE FileHandle = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, 
				   0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE) {

	LARGE_INTEGER FileSize;
	if(GetFileSizeEx(FileHandle, &FileSize)) {

	    // TODO Define for maximum values e.g. UInt32Max
	    uint32_t FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
	    Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		if (Result.Contents) {
			DWORD BytesRead;
			if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
					(BytesRead == FileSize32)) {
				Result.ContentsSize = FileSize32;

			} else {
				DEBUGPlatformFreeFileMemory(Result.Contents);
				Result.Contents = 0;
			}
		} else {
			// TODO Logging
		}
	} else {
		// TODO Logging
	}

	CloseHandle(FileHandle);
	} else {
		// TODO Logging
	}

	return Result;
}

internal void DEBUGPlatformFreeFileMemory(void *Memory) {
    if (Memory) {
	VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint64_t MemorySize, void *Memory) {

	bool32 Result = false;

	HANDLE FileHandle = CreateFile(Filename, GENERIC_WRITE, 0, 
			0, CREATE_ALWAYS, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE) {

		DWORD BytesWritten;
		if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0)) {

			Result = (BytesWritten == MemorySize);

		} else {
			// TODO Logging
		}
		CloseHandle(FileHandle);

	} else {
		// TODO Logging
	}

	return Result;
}
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
    // TODO Aspect ratio correction
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

internal void Win32ClearBuffer(win32_sound_output *SoundOutput) {

    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, 
						&Region1, &Region1Size,
						&Region2, &Region2Size,
						0))) {

	uint8_t *DestSample = (uint8_t *)Region1;
        for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)  {
	    
            *DestSample++ = 0;
        }

	DestSample = (uint8_t *)Region2;
        for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)  {
	    
            *DestSample++ = 0;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }


}
internal void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, 
				   game_sound_output_buffer *SourceBuffer) {
    
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, 
						&Region1, &Region1Size,
						&Region2, &Region2Size,
						     0))) {

        // TODO assert that RegionSize1/RegionSize2 is valid
        int16_t *DestSample = (int16_t *)Region1;
	int16_t *SourceSample = SourceBuffer->Samples;
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)  {

            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;

        }

        DestSample = (int16_t *)Region2;
        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)  {

            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);

    }   
}

internal void 
Win32ProcessXInputDigitalButton(DWORD XInputButtonState, DWORD ButtonBit, 
				game_button_state *NewState, game_button_state *OldState ) {

    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}
int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode) {
    
    uint64_t LastCycleCount = __rdtsc();
    LARGE_INTEGER PerfCounterFrequencyResult;
    QueryPerformanceFrequency(&PerfCounterFrequencyResult);
    int64_t PerfCounterFrequency = PerfCounterFrequencyResult.QuadPart;

    Win32LoadXInput();

    // Making a window class
    WNDCLASS WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc= Win32MainWindowCallback;
    WindowClass.hInstance= Instance;
    WindowClass.lpszClassName=TEXT("HandmadeHeroWindowClass");
    // WindowClass.hIcon
    
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
	    SoundOutput.RunningSampleIndex = 0;
	    SoundOutput.BytesPerSample = sizeof(int16_t)*2;
	    SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
    
	    Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
	    Win32ClearBuffer(&SoundOutput);
	    GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

	    LARGE_INTEGER LastCounter;
	    QueryPerformanceCounter(&LastCounter);

	    // TODO Pool with bitmap VirtualAlloc
	    int16_t *Samples = 
		(int16_t *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, 
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#if HANDMADE_INTERNAL
	    LPVOID BaseAddress = (LPVOID)Terabytes((uint64_t)2);
#else
	    LPVOID BaseAddress = 0;
#endif
	    game_memory GameMemory = {};
	    GameMemory.PermanentStorageSize = Megabytes(64);
	    GameMemory.TransientStorageSize = Gigabytes((uint64_t)4);

	    uint64_t TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
	    GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, TotalSize,
							MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	    GameMemory.TransientStorage = ((uint8_t *)GameMemory.PermanentStorage + 
					    GameMemory.PermanentStorageSize);

	    if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage) {
	    
		game_input Input[2] = {};
		game_input *NewInput = &Input[0];
		game_input *OldInput = &Input[1];

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
		    int MaxControllerCount = XUSER_MAX_COUNT;
		    if (MaxControllerCount > ArrayCount(OldInput->Controllers)) {

			MaxControllerCount = ArrayCount(NewInput->Controllers);
		    }
		    for (DWORD ControllerIndex = 0; 
			    ControllerIndex < MaxControllerCount; 
			    ++ControllerIndex) {

			game_controller_input *OldController = &OldInput->Controllers[ControllerIndex];
			game_controller_input *NewController = &NewInput->Controllers[ControllerIndex];
			XINPUT_STATE ControllerState;
			if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {

			    //Controller Connected
			    // TODO See the behavior of dwPacketNumber
			    XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;	

			    // TODO DPad
			    bool32 Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
			    bool32 Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
			    bool32 Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
			    bool32 Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

			    NewController->IsAnalog = true;
			    NewController->StartX = OldController->EndX;
			    NewController->StartY = OldController->EndY;

			    // TODO Min/Max Macros?
			    real32 X;
			    if (Pad->sThumbLX < 0) {
				X = (real32)Pad->sThumbLX / 32768.0f;
			    } else {
				X = (real32)Pad->sThumbLX / 32767.0f;
			    }
			    NewController->MinX = NewController->MaxX = NewController->EndX = X;

			    real32 Y;
			    if (Pad->sThumbLY < 0) {
				Y = (real32)Pad->sThumbLY / 32768.0f;
			    } else {
				Y = (real32)Pad->sThumbLY / 32767.0f;
			    }
			    NewController->MinX = NewController->MaxX = NewController->EndX = X;
			    NewController->MinY = NewController->MaxY = NewController->EndY = Y;
			    int16_t StickX = Pad->sThumbLX;
			    int16_t StickY = Pad->sThumbLY;

			    // TODO Handle Deadzones

			    Win32ProcessXInputDigitalButton(Pad->wButtons, 
				    XINPUT_GAMEPAD_A, 
				    &OldController->Down, 
				    &NewController->Down);

			    Win32ProcessXInputDigitalButton(Pad->wButtons, 
				    XINPUT_GAMEPAD_B, 
				    &OldController->Left, 
				    &NewController->Left);

			    Win32ProcessXInputDigitalButton(Pad->wButtons, 
				    XINPUT_GAMEPAD_X, 
				    &OldController->Left, 
				    &NewController->Left);

			    Win32ProcessXInputDigitalButton(Pad->wButtons, 
				    XINPUT_GAMEPAD_Y, 
				    &OldController->Up, 
				    &NewController->Up);

			    Win32ProcessXInputDigitalButton(Pad->wButtons, 
				    XINPUT_GAMEPAD_LEFT_SHOULDER, 
				    &OldController->LeftShoulder, 
				    &NewController->LeftShoulder);

			    Win32ProcessXInputDigitalButton(Pad->wButtons, 
				    XINPUT_GAMEPAD_RIGHT_SHOULDER, 
				    &OldController->RightShoulder, 
				    &NewController->RightShoulder);

			    //bool32 Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
			    //bool32 Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
			    //bool32 LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			    //bool32 RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
			    //bool32 AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
			    //bool32 BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
			    //bool32 XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
			    //bool32 YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);


			    // TODO handle deadzone for the thumb sticks


			} else {

			    // Controller Not Available
			}
		    }

		    XINPUT_VIBRATION Vibration;
		    Vibration.wLeftMotorSpeed = 60000;
		    Vibration.wRightMotorSpeed = 60000;
		    // XInputSetState(0, &Vibration);

		    DWORD BytesToWrite;
		    DWORD TargetCursor;
		    DWORD ByteToLock;
		    DWORD PlayCursor;
		    DWORD WriteCursor;
		    bool32 SoundIsValid = false;
		    // TODO Tighten the sound logic so that we know where we should be writing to and can
		    // anticipate the time spent in the game update.
		    if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))) {

			ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) % 
				SoundOutput.SecondaryBufferSize);

			TargetCursor = (PlayCursor +
				(SoundOutput.LatencySampleCount*SoundOutput.BytesPerSample)) % 
			    SoundOutput.SecondaryBufferSize;

			// TODO change this to a lower latency offset from the playcursor when we have FX
			if (ByteToLock > TargetCursor) {
			    BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
			    BytesToWrite += TargetCursor;
			} else {
			    BytesToWrite = TargetCursor - ByteToLock;
			}

			SoundIsValid = true;
		    }



		    game_sound_output_buffer SoundBuffer {};
		    SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond; 
		    SoundBuffer.SampleCountToOutput = BytesToWrite / SoundOutput.BytesPerSample;
		    SoundBuffer.Samples = Samples;

		    game_offscreen_buffer Buffer = {};
		    Buffer.Memory = GlobalBackBuffer.Memory;
		    Buffer.Width  = GlobalBackBuffer.Width; 
		    Buffer.Height = GlobalBackBuffer.Height;
		    Buffer.Pitch  = GlobalBackBuffer.Pitch;
		    Buffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;

		    GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);

		    // DirectSound Output test
		    if (SoundIsValid) {

			// TODO Test Required
			Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer); 
		    }

		    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
		    HDC DeviceContext = GetDC(Window);
		    Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, 
			    Dimension.Width, Dimension.Height);
		    ReleaseDC(Window, DeviceContext);

		    uint64_t EndCycleCount = __rdtsc();

		    // TODO Display the value here
		    LARGE_INTEGER EndCounter;
		    QueryPerformanceCounter(&EndCounter);

		    uint64_t CyclesElapsed = EndCycleCount - LastCycleCount;
		    int64_t CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
		    int32_t MSPerFrame = (int32_t)((1000*CounterElapsed)/PerfCounterFrequency);
		    int32_t FPS = PerfCounterFrequency / CounterElapsed;
		    int32_t MCPS = (int32_t)CyclesElapsed / (1000 * 1000);

#if 0
		    char Buffer[256];
		    wsprintf(Buffer, "%dms/f, %df/s, %dmc/f\n", MSPerFrame, FPS, MCPS);
		    OutputDebugStringA(Buffer);
#endif

		    LastCounter = EndCounter;
		    LastCycleCount = EndCycleCount;

		    game_input *Temp = NewInput;
		    NewInput = OldInput;
		    OldInput = Temp;
		    // TODO should we clear this here
		}
	    } else {
		// TODO Memory cannot be initialized
	    }

        } else {

        }

    } else {

    }

    return 0;
}
