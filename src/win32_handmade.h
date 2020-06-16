#if !defined(WIN32_HANDMADE_H)
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

struct win32_sound_output {
    
    int SamplesPerSecond; 
    uint32_t RunningSampleIndex;
    int BytesPerSample;
    DWORD SecondaryBufferSize;
    real32 tSine;
    int LatencySampleCount;
	DWORD SafetyBytes;
};

struct win32_debug_time_marker
{
	DWORD OutputPlayCursor;
	DWORD OutputWriteCursor;
	DWORD OutputLocation;
	DWORD OutputByteCount;
	DWORD ExpectedFlipPlayCursor;

	DWORD FlipPlayCursor;
	DWORD FlipWriteCursor;
};

struct win32_game_code
{
	HMODULE GameCodeDLL;
	FILETIME DLLLastWriteTime;
	game_update_and_render *UpdateAndRender;
	game_get_sound_samples *GetSoundSamples;

	bool32 IsValid;
};
#define WIN32_HANDMADE_H
#endif
