/*
   NOTE THIS IS NOT A FINAL PLATFORM LAYER

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

#include "handmade.h"

#include <stdio.h>
#include <windows.h>
#include <xinput.h>
#include <dsound.h>

#include "win32_handmade.h"

// TODO have to change this from global, initialized to zero by default
global_var bool8 GlobalRunning;
global_var win32_offscreen_buffer GlobalBackBuffer; 
global_var LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_var int64_t GlobalPerfCounterFrequency;

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

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
	if (Memory) {
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{

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

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
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

inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
	FILETIME LastWriteTime = {};

	WIN32_FIND_DATA FindData;
	HANDLE FindHandle = FindFirstFile(Filename, &FindData);
	if (FindHandle != INVALID_HANDLE_VALUE)
	{
		LastWriteTime = FindData.ftLastWriteTime;
		FindClose(FindHandle);
	}
	return LastWriteTime;
}

internal win32_game_code Win32LoadGameCode(char *SourceDLLName, char *TempDLLName)
{
	win32_game_code Result = {};

	Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
	
	CopyFile(SourceDLLName, TempDLLName, FALSE);
	Result.GameCodeDLL = LoadLibraryA(TempDLLName);
	if(Result.GameCodeDLL)
	{
		Result.UpdateAndRender = 
			(game_update_and_render *)GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
		Result.GetSoundSamples = 
			(game_get_sound_samples *)GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples");
	}

	Result.IsValid = (Result.UpdateAndRender && Result.GetSoundSamples);

	if (!Result.IsValid)
	{
		Result.UpdateAndRender = GameUpdateAndRenderStub;
		Result.GetSoundSamples = GameGetSoundSamplesStub;
	}
	return Result;
}

internal void
Win32UnloadGameCode (win32_game_code *GameCode)
{
	if(GameCode->GameCodeDLL)
	{
		FreeLibrary(GameCode->GameCodeDLL);
	}

	GameCode->IsValid = false;
	GameCode->UpdateAndRender =GameUpdateAndRenderStub;
	GameCode->GetSoundSamples =GameGetSoundSamplesStub; 
}

internal void Win32LoadXInput(void) {

	// TODO Diagnostic
	HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
	if(!XInputLibrary) {

		HMODULE XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}

	if(!XInputLibrary) {

		HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
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
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
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
		Assert(!"Keyboard event came in through a non-dispatch message!");
	} break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);

            //int X = Paint.rcPaint.left;
            //int Y = Paint.rcPaint.top;
            //int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            //int Width = Paint.rcPaint.right - Paint.rcPaint.left;

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
Win32ProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown) {

	Assert(NewState->EndedDown != IsDown);
	NewState->EndedDown = IsDown;
	++NewState->HalfTransitionCount;
}

internal void 
Win32ProcessXInputDigitalButton(DWORD XInputButtonState, DWORD ButtonBit, 
		game_button_state *NewState, game_button_state *OldState ) {

	NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
	NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

internal real32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold) {
	
	real32 Result = 0;
	if (Value < -DeadZoneThreshold) {
		Result = (real32)(Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold);
	} else if (Value > DeadZoneThreshold) {
		Result = (real32)(Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold);
	}
	return Result;
}

internal void
Win32ProcessPendingMessages(game_controller_input *KeyboardController) {

	MSG Message;
	while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {

		if(Message.message == WM_QUIT) {
			GlobalRunning = 0;
		}

		switch (Message.message) {

			case WM_QUIT:
			{
				GlobalRunning = false;
			} break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				uint32_t VKCode = (uint32_t)Message.wParam;
				bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
				bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);

				if (WasDown != IsDown) {

					if(VKCode == 'W') {
						Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown); 
					}
					else if(VKCode == 'A') {
						Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown); 
					}
					else if(VKCode == 'S') {
						Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown); 
					}
					else if(VKCode == 'D') {
						Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown); 
					}
					else if(VKCode == 'E') {
						Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown); 
					}
					else if(VKCode == 'Q') {
						Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown); 
					}
					else if(VKCode == VK_UP) {
						Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown); 
					}
					else if(VKCode == VK_DOWN) {
						Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown); 
					}
					else if(VKCode == VK_RIGHT) {
						Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown); 
					}
					else if(VKCode == VK_LEFT) {
						Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown); 
					}
					else if(VKCode == VK_ESCAPE) {
						Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown); 
					}
					else if(VKCode == VK_SPACE) {
						Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown); 
					}

					bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
					if((VKCode == VK_F4) && AltKeyWasDown) {

						GlobalRunning = 0;
					}
				}
			} break;

			default:
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			} break;
		}
	}
}


inline LARGE_INTEGER Win32GetWallClock(void)
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return Result;
}

inline real32 Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	real32 Result = ((real32)(End.QuadPart - Start.QuadPart) /
					 (real32)GlobalPerfCounterFrequency);
	return Result;
}

internal void
Win32DebugDrawVertical (win32_offscreen_buffer *Buffer, 
						int X, int Top, int Bottom, uint32_t Color)
{
	if (Top <= 0)
	{
		Top = 0;
	}
	if (Bottom > Buffer->Height)
	{
		Bottom = Buffer->Height;
	}

	if ((X >= 0) && (X < Buffer->Width))
	{
		uint8_t *Pixel = ((uint8_t *)Buffer->Memory +
				X*Buffer->BytesPerPixel +
				Top*Buffer->Pitch);
		for (int Y = Top;
				Y < Bottom;
				++Y)
		{
			*(uint32_t *)Pixel = Color;
			Pixel += Buffer->Pitch;
		}	
	}

}

inline void
Win32DrawSoundBufferMarker(win32_offscreen_buffer *Buffer,
						   win32_sound_output *SoundOutput,
						   real32 C, int PadX, int Top, int Bottom,
						   DWORD Value, uint32_t Color)
{
	real32 XReal32 = (C * (real32)Value);
	int X = PadX + (int)XReal32;
	Win32DebugDrawVertical(Buffer, X, Top, Bottom, Color);
}

internal void 
Win32DebugSyncDisplay(win32_offscreen_buffer *BackBuffer,
					  int MarkerCount,
					  win32_debug_time_marker * Markers, 
					  int CurrentMarkerIndex,
					  win32_sound_output *SoundOutput,
					  real32 TargetSecondsPerFrame)
{
	int PadX = 16;
	int PadY = 16;

	int LineHeight = 64;

	real32 C = (real32)(BackBuffer->Width - 2*PadX) / (real32)SoundOutput->SecondaryBufferSize;
	for (int MarkerIndex = 0;
		 MarkerIndex < MarkerCount;
		 ++MarkerIndex)
	{
		win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];

		Assert(ThisMarker->OutputPlayCursor < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->OutputWriteCursor < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->OutputLocation < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->OutputByteCount < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->FlipPlayCursor < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->FlipWriteCursor < SoundOutput->SecondaryBufferSize);

		DWORD PlayColor = 0xFFFFFFFF;
		DWORD WriteColor = 0xFFFF0000;
		DWORD ExpectedFlipColor = 0xFFFFFF00;
		
		int Top = PadY;
		int Bottom = PadY + LineHeight;
		if (MarkerIndex == CurrentMarkerIndex)
		{
			//PlayColor =;
			//WriteColor =;
			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;
			int FirstTop = Top;
			Win32DrawSoundBufferMarker (BackBuffer, SoundOutput, C, PadX, Top, Bottom, 
										ThisMarker->OutputPlayCursor, PlayColor);
			Win32DrawSoundBufferMarker (BackBuffer, SoundOutput, C, PadX, Top, Bottom, 
										ThisMarker->OutputWriteCursor, WriteColor);


			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;
			Win32DrawSoundBufferMarker (BackBuffer, SoundOutput, C, PadX, Top, Bottom, 
										ThisMarker->OutputLocation, PlayColor);
			Win32DrawSoundBufferMarker (BackBuffer, SoundOutput, C, PadX, Top, Bottom, 
										ThisMarker->OutputLocation + ThisMarker->OutputByteCount, WriteColor);

			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;

			Win32DrawSoundBufferMarker (BackBuffer, SoundOutput, C, PadX, FirstTop, Bottom, 
										ThisMarker->ExpectedFlipPlayCursor, ExpectedFlipColor);
		}
		Win32DrawSoundBufferMarker (BackBuffer, SoundOutput, C, PadX, Top, Bottom, 
									ThisMarker->FlipPlayCursor, PlayColor);
		Win32DrawSoundBufferMarker (BackBuffer, SoundOutput, C, PadX, Top, Bottom, 
									ThisMarker->FlipWriteCursor, WriteColor);
	}
}

internal void CatStrings(size_t SourceACount, char *SourceA,
						 size_t SourceBCount, char *SourceB,
						 size_t DestCount, char *Dest)
{
	// TODO Check Dest bounds for overflow
	for(int Index = 0;
			Index < SourceACount;
			++Index)
	{
		*Dest++ = *SourceA++;
	}

	for(int Index = 0;
			Index < SourceBCount;
			++Index)
	{
		*Dest++ = *SourceB++;
	}
	*Dest++ = 0;
}

int CALLBACK WinMain(
		HINSTANCE Instance,
		HINSTANCE PrevInstance,
		LPSTR CommandLine,
		int ShowCode) {

	// WARNING MAX_PATH is no longer the final max path of files, 
	// and should not be used in the shipping code
	char EXEFileName[MAX_PATH];
	DWORD SizeOfFilename = GetModuleFileNameA(0, EXEFileName, sizeof(EXEFileName));
	char *OnePastLastSlash = EXEFileName + SizeOfFilename;
	for (char *Scan = EXEFileName;
		 *Scan;
		 ++Scan)
	{
		if(*Scan == '\\')
		{
			OnePastLastSlash = Scan + 1;
		}
	}

	char SourceGameCodeDLLFileName[] = "handmade.dll";
	char SourceGameCodeDLLFullPath[MAX_PATH];
	CatStrings (OnePastLastSlash - EXEFileName, EXEFileName,
				sizeof(SourceGameCodeDLLFileName) - 1, SourceGameCodeDLLFileName,
				sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);

	char TempGameCodeDLLFileName[] = "handmade_temp.dll";
	char TempGameCodeDLLFullPath[MAX_PATH];
	CatStrings (OnePastLastSlash - EXEFileName, EXEFileName,
				sizeof(TempGameCodeDLLFileName) - 1, TempGameCodeDLLFileName,
				sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

	LARGE_INTEGER PerfCounterFrequencyResult;
	QueryPerformanceFrequency(&PerfCounterFrequencyResult);
	GlobalPerfCounterFrequency = PerfCounterFrequencyResult.QuadPart;

	// NOTE Set the Windows scheduler granularity to 1ms
	// so that our sleep can be more granular.
	UINT DesiredSchedulerMS = 1;
	bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

	Win32LoadXInput();

	// Making a window class
	WNDCLASS WindowClass = {};

	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	WindowClass.style = CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc= Win32MainWindowCallback;
	WindowClass.hInstance= Instance;
	WindowClass.lpszClassName=TEXT("HandmadeHeroWindowClass");
	// WindowClass.hIcon

	// TODO how do we reliably query this on Windows?

#define	MonitorRefreshHz 60
#define	GameUpdateHz (MonitorRefreshHz / 2)
	real32 TargetSecondsPerFrame = 1.0f / (real32)GameUpdateHz;

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

			HDC DeviceContext = GetDC(Window);

			// Sound Variables
			win32_sound_output SoundOutput = {};
			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.BytesPerSample = sizeof(int16_t)*2;
			SoundOutput.LatencySampleCount = 
				3 * (SoundOutput.SamplesPerSecond / GameUpdateHz);
			SoundOutput.RunningSampleIndex = 0;
			SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
			// TODO Actually compute this value and see what the lowest reasonable value is.
			SoundOutput.SafetyBytes = 
				(SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample / GameUpdateHz) / 5;

			Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
			Win32ClearBuffer(&SoundOutput);
			GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
			// NOTE this tests the Playcursor/WriteCursor update frequency, last check was 480 Samples
#if 0
			GlobalRunning = 1;
			while(GlobalRunning)
			{
				DWORD PlayCursor;
				DWORD WriteCursor;
				GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor);
					char TextBuffer[256];
				_snprintf_s(TextBuffer, sizeof(TextBuffer), 
							"PC: %u, WC: %u\n", PlayCursor, WriteCursor);
				OutputDebugStringA(TextBuffer);
			}
#endif

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
			GameMemory.TransientStorageSize = Gigabytes(1);
			GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
			GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
			GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
			uint64_t TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
			GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize,
					MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			GameMemory.TransientStorage = ((uint8_t *)GameMemory.PermanentStorage + 
					GameMemory.PermanentStorageSize);

			if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage) {

				DWORD DebugPlayCursor = 0;

				game_input Input[2] = {};
				game_input *NewInput = &Input[0];
				game_input *OldInput = &Input[1];

				LARGE_INTEGER FlipWallClock = Win32GetWallClock();
				LARGE_INTEGER LastCounter = Win32GetWallClock();

				int DebugTimeMarkersIndex = 0;
				win32_debug_time_marker DebugTimeMarkers[GameUpdateHz / 2] = {0};

				bool32 SoundIsValid = false;
				DWORD AudioLatencyBytes = 0;
				real32 AudioLatencySeconds = 0;

				win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, 
														 TempGameCodeDLLFullPath);

				uint64_t LastCycleCount = __rdtsc();
				GlobalRunning = 1;
				while(GlobalRunning) {

					FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
					if (CompareFileTime(&NewDLLWriteTime, &Game.DLLLastWriteTime) != 0)
					{
						Win32UnloadGameCode(&Game);
						Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, 
												 TempGameCodeDLLFullPath);
					}

					// TODO Zeroing Macros
					// TODO We can't zero everything because the up/down state will be wrong
					game_controller_input *OldKeyboardController = GetController(OldInput, 0);
					game_controller_input *NewKeyboardController = GetController(NewInput, 0);
					game_controller_input ZeroController = {};
					*NewKeyboardController = ZeroController;
					NewKeyboardController->IsConnected = true;
					for (int ButtonIndex = 0;
						ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
						++ButtonIndex) {

						NewKeyboardController->Buttons[ButtonIndex].EndedDown = 
							OldKeyboardController->Buttons[ButtonIndex].EndedDown;
					}
					Win32ProcessPendingMessages(NewKeyboardController);

					// TODO need to not pull the disconnected controllers to avoid xinput frame rate hit
					// on older libraries.

					// TODO should we pull this more frequently
					DWORD MaxControllerCount = XUSER_MAX_COUNT;
					if (MaxControllerCount > (ArrayCount(OldInput->Controllers) - 1)) {

						MaxControllerCount = (ArrayCount(NewInput->Controllers) - 1);
					}
					for (DWORD ControllerIndex = 0; 
						ControllerIndex < MaxControllerCount; 
						++ControllerIndex) {

						DWORD OurControllerIndex = ControllerIndex + 1;
						game_controller_input *OldController = GetController(OldInput, OurControllerIndex);
						game_controller_input *NewController = GetController(NewInput, OurControllerIndex);
						XINPUT_STATE ControllerState;
						if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {

							//Controller Connected
							NewController->IsConnected = true;	
							// TODO See the behavior of dwPacketNumber
							XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;	

							// TODO This is a square deadzone, check XInput to verify that the deadzone
							// is circular or not.
							NewController->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX,
																		XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							NewController->StickAverageY = Win32ProcessXInputStickValue(Pad->sThumbLY,
																		XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							if ((NewController->StickAverageX != 0.0f) || 
								(NewController->StickAverageY != 0.0f))
							{
								NewController->IsAnalog = true;
							}

							// DPad
							if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
							{
								NewController->StickAverageY = 1.0f;
								NewController->IsAnalog = false;
							}

							if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
							{
								NewController->StickAverageY = -1.0f;
								NewController->IsAnalog = false;
							}

							if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
							{
								NewController->StickAverageX = 1.0f;
								NewController->IsAnalog = false;
							}

							if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
							{
								NewController->StickAverageX = -1.0f;
								NewController->IsAnalog = false;
							}

							real32 Threshold = 0.5f;
							Win32ProcessXInputDigitalButton(
									(NewController->StickAverageX) < -Threshold ? 1 : 0, 
									1, 
									&OldController->MoveLeft, 
									&NewController->MoveLeft);

							Win32ProcessXInputDigitalButton(
									(NewController->StickAverageX) > Threshold ? 1 : 0, 
									1, 
									&OldController->MoveRight, 
									&NewController->MoveRight);

							Win32ProcessXInputDigitalButton(
									(NewController->StickAverageY) < -Threshold ? 1 : 0, 
									1, 
									&OldController->MoveDown, 
									&NewController->MoveDown);

							Win32ProcessXInputDigitalButton(
									(NewController->StickAverageY) > Threshold ? 1 : 0, 
									1, 
									&OldController->MoveUp, 
									&NewController->MoveUp);

							Win32ProcessXInputDigitalButton(Pad->wButtons, 
									XINPUT_GAMEPAD_A, 
									&OldController->ActionDown, 
									&NewController->ActionDown);

							Win32ProcessXInputDigitalButton(Pad->wButtons, 
									XINPUT_GAMEPAD_B, 
									&OldController->ActionRight, 
									&NewController->ActionRight);

							Win32ProcessXInputDigitalButton(Pad->wButtons, 
									XINPUT_GAMEPAD_X, 
									&OldController->ActionLeft, 
									&NewController->ActionLeft);

							Win32ProcessXInputDigitalButton(Pad->wButtons, 
									XINPUT_GAMEPAD_Y, 
									&OldController->ActionUp, 
									&NewController->ActionUp);

							Win32ProcessXInputDigitalButton(Pad->wButtons, 
									XINPUT_GAMEPAD_LEFT_SHOULDER, 
									&OldController->LeftShoulder, 
									&NewController->LeftShoulder);

							Win32ProcessXInputDigitalButton(Pad->wButtons, 
									XINPUT_GAMEPAD_RIGHT_SHOULDER, 
									&OldController->RightShoulder, 
									&NewController->RightShoulder);

							Win32ProcessXInputDigitalButton(Pad->wButtons, 
									XINPUT_GAMEPAD_START, 
									&OldController->Start, 
									&NewController->Start);

							Win32ProcessXInputDigitalButton(Pad->wButtons, 
									XINPUT_GAMEPAD_BACK, 
									&OldController->Back, 
									&NewController->Back);

							// TODO handle deadzone for the thumb sticks


						} else {

							// Controller Not Available
							NewController->IsConnected = false;	
						}
					}

					//XINPUT_VIBRATION Vibration;
					//Vibration.wLeftMotorSpeed = 60000;
					//Vibration.wRightMotorSpeed = 60000;
					// XInputSetState(0, &Vibration);

					game_offscreen_buffer Buffer = {};
					Buffer.Memory = GlobalBackBuffer.Memory;
					Buffer.Width  = GlobalBackBuffer.Width; 
					Buffer.Height = GlobalBackBuffer.Height;
					Buffer.Pitch  = GlobalBackBuffer.Pitch;
					Buffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;

					Game.UpdateAndRender(&GameMemory, NewInput, &Buffer);

					LARGE_INTEGER AudioWallClock = Win32GetWallClock();
					real32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);

					DWORD PlayCursor;
					DWORD WriteCursor;
					if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
					{
						/*
							NOTE: Here is how the sound output computation works.

							We define a safety value that is the number of samples we think our game update
							loop may vary by (maybe by up to 2ms).

							When we wake up to write the audio, we will look and see where the playCursor
							position is and we will forecast ahead where we think the play cursor will be
							on the next frame boundary.

							We will then look to see if the write cursor is before that by our safety value.
					   		If it is, then the target fill position is that frame boundary plus one one 
							frame giving us perfect audio sync in case of a card that has a low enough 
							latency.

							if the write cursor is after the that safety margin boundary, then we assume 
							we can never sync the audio perfectly, so we will write one frame of audio 
							plus the safety value's worth of guard samples. 						
						*/
#if HANDMADE_INTERNAL
						win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkersIndex];
#endif

						if (!SoundIsValid)
						{
							SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
							SoundIsValid = true;
						}

						DWORD ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) % 
											 SoundOutput.SecondaryBufferSize);

						DWORD ExpectedSoundBytesPerFrame = 
							(SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample) / GameUpdateHz;
						
						real32 SecondsLeftUntilFlip = (TargetSecondsPerFrame - FromBeginToAudioSeconds);
						DWORD ExpectedBytesUntilFlip = 
							(DWORD)((SecondsLeftUntilFlip/TargetSecondsPerFrame) * 
									(real32)ExpectedSoundBytesPerFrame);

						// WARNING Casey hasn't done this
						DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;

						DWORD SafeWriteCursor = WriteCursor;
						if (SafeWriteCursor < PlayCursor)
						{
							SafeWriteCursor += SoundOutput.SecondaryBufferSize;
						}
						Assert(SafeWriteCursor >= PlayCursor);
					   	SafeWriteCursor += SoundOutput.SafetyBytes;

						bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);
						DWORD TargetCursor = 0;
						if(AudioCardIsLowLatency)
						{
							TargetCursor = (ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame);
						}
						else
						{
							TargetCursor = 
								(WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes);
						}
						
						TargetCursor = TargetCursor % SoundOutput.SecondaryBufferSize;

						DWORD BytesToWrite = 0;
						if (ByteToLock > TargetCursor) {
							BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
							BytesToWrite += TargetCursor;
						} else {
							BytesToWrite = TargetCursor - ByteToLock;
						}
						game_sound_output_buffer SoundBuffer {};
						SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond; 
						SoundBuffer.SampleCountToOutput = BytesToWrite / SoundOutput.BytesPerSample;
						SoundBuffer.Samples = Samples;
						Game.GetSoundSamples(&GameMemory, &SoundBuffer);

#if HANDMADE_INTERNAL
						Marker->OutputPlayCursor = PlayCursor;
						Marker->OutputWriteCursor = WriteCursor;
						Marker->OutputLocation = ByteToLock;
						Marker->OutputByteCount = BytesToWrite;
						Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;

						DWORD UnWrappedWriteCursor = WriteCursor;
						if (UnWrappedWriteCursor < PlayCursor)
						{
							DWORD UnWrappedWriteCursor = SoundOutput.SecondaryBufferSize;
						}
						AudioLatencyBytes = UnWrappedWriteCursor - PlayCursor;
						AudioLatencySeconds = 
							(((real32)AudioLatencyBytes / (real32)SoundOutput.BytesPerSample) /
							 (real32)SoundOutput.SamplesPerSecond);

						char TextBuffer[256];
						_snprintf_s(TextBuffer, sizeof(TextBuffer), 
								"BTL: %u, TC: %u, BTW: %u - PC: %u, WC: %u DELTA: %u (%fs)\n",
								ByteToLock, TargetCursor, BytesToWrite, 
								PlayCursor, WriteCursor, AudioLatencyBytes, AudioLatencySeconds);
						OutputDebugStringA(TextBuffer);
#endif
						// TODO Test Required
						Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer); 
					}
					else
					{
						SoundIsValid = false;
					}

					// TODO Display the value here
					
					LARGE_INTEGER WorkCounter = Win32GetWallClock();
					real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

					// TODO Not tested yet, probably buggy
					real32 SecondsElapsedForFrame = WorkSecondsElapsed;
					if (SecondsElapsedForFrame < TargetSecondsPerFrame)
					{
						if(SleepIsGranular)
						{
							DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
							if (SleepMS > 0)
							{
								Sleep(SleepMS);
							}
						}

						Assert(SecondsElapsedForFrame < TargetSecondsPerFrame);

						while(SecondsElapsedForFrame < TargetSecondsPerFrame)
						{
							SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
						}
					}
					else
					{
						// WARNING missed framerate
						// TODO must handle it 
					}

					LARGE_INTEGER EndCounter = Win32GetWallClock();
					real32 MSPerFrame = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
					LastCounter = EndCounter;

					win32_window_dimension Dimension = Win32GetWindowDimension(Window);

#if HANDMADE_INTERNAL
					// TODO this is wrong for the zero'th index
					Win32DebugSyncDisplay(&GlobalBackBuffer, ArrayCount(DebugTimeMarkers), 
										  DebugTimeMarkers, DebugTimeMarkersIndex - 1, 
										  &SoundOutput, TargetSecondsPerFrame);
#endif
					Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, 
												Dimension.Width, Dimension.Height);

					FlipWallClock = Win32GetWallClock();
#if HANDMADE_INTERNAL
					{
						DWORD PlayCursor;
						DWORD WriteCursor;
						if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
						{
							Assert(DebugTimeMarkersIndex < ArrayCount(DebugTimeMarkers));
							win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkersIndex];

							Marker->FlipPlayCursor = PlayCursor;
							Marker->FlipWriteCursor = WriteCursor;
						}
					}
#endif
					game_input *Temp = NewInput;
					NewInput = OldInput;
					OldInput = Temp;

					uint64_t EndCycleCount = __rdtsc();
					uint64_t CyclesElapsed = EndCycleCount - LastCycleCount;
					LastCycleCount = EndCycleCount;

					real64 FPS = 0.0f;
					real64 MCPS = (int32_t)CyclesElapsed / (1000 * 1000);

					char FPSBuffer[256];
					_snprintf_s(FPSBuffer, sizeof(FPSBuffer), 
								"%.02fms/f, %.02ff/s, %.02fmc/f\n", MSPerFrame, FPS, MCPS);
					//OutputDebugStringA(FPSBuffer);
#if HANDMADE_INTERNAL
					++DebugTimeMarkersIndex;
					if(DebugTimeMarkersIndex == ArrayCount(DebugTimeMarkers))
					{
						DebugTimeMarkersIndex = 0;
					}
#endif
				}
			} else {
				// TODO Memory cannot be initialized
			}
			
			ReleaseDC(Window, DeviceContext);

		} else {

		}

	} else {

	}
	return 0;
}
