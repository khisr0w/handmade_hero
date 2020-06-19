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

// This is the header file for the platform independent layer
#if !defined(HANDMADE_H)

/*
	HANDMADE_INTERNAL:
	0 - Build for the public release
	1 - Build for developer only

	HANDMADE_SLOW:
	0 - No Slow Code allowed
	1 - Slow code allowed
*/

#if HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int*)0 = 0;}
#else
#define Assert(Expression)
#endif

inline uint32_t SafeTruncateUInt64 (uint64_t Value) {

	Assert(Value <= 0xFFFFFFFF);
	return (uint32_t)Value;
}

struct thread_context
{
	int Placeholder;
};

// TODO should this always be 64-bit?
#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

/* 
   TODO Services that the platform layer provides to the game
*/

#if HANDMADE_INTERNAL
/*
   NOTE These are not for doing anything in the shipping game - they are blocking and the write doesn't
   protect against lost data!
*/

struct debug_read_file_result {
	uint32_t ContentsSize;
	void* Contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, \
	   																	  char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name (thread_context *Thread, char *Filename, \
															uint32_t MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

/* 
   Services that the game provide to the platform layer
*/

// FOUR THINGS - Timing, Controller/Keyboard, Bitmap buffer to use, Sound Buffer to use

struct game_sound_output_buffer {

	int SampleCountToOutput;
	int16_t *Samples;
	int SamplesPerSecond; 
};

struct game_offscreen_buffer {

	void *Memory;
	int Width; 
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct game_button_state {

	int HalfTransitionCount;
	bool32 EndedDown;
};

struct game_controller_input {

	bool32 IsConnected;
	bool32 IsAnalog;
	real32 StickAverageX;
	real32 StickAverageY;

	union {
		game_button_state Buttons[12];
		struct {
			game_button_state MoveUp;
			game_button_state MoveDown;
			game_button_state MoveLeft;
			game_button_state MoveRight;

			game_button_state ActionUp;
			game_button_state ActionDown;
			game_button_state ActionLeft;
			game_button_state ActionRight;

			game_button_state LeftShoulder;
			game_button_state RightShoulder;

			game_button_state Start;
			game_button_state Back;

			// WARNING All buttons must be added above this line, the Terminator button must not
			// be used except for assertions
			game_button_state Terminator;
		};
	};
};

struct game_input 
{
	game_button_state MouseButtons[5];
	int32_t MouseX, MouseY, MouseZ;
	// TODO Insert game clock values in here
	game_controller_input Controllers[5];
};

inline game_controller_input *GetController (game_input *Input, int unsigned ControllerIndex)
{
	Assert(ControllerIndex < ArrayCount(Input->Controllers));

	game_controller_input *Result = &Input->Controllers[ControllerIndex];
	return Result;
}

struct game_memory {
	bool32 IsInitialized;

	uint64_t PermanentStorageSize;
	void *PermanentStorage; // This memory is required to be zero at startup

	uint64_t TransientStorageSize;
	void *TransientStorage; // This memory is required to be zero at startup

	debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
	debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
	debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
};


#define GAME_UPDATE_AND_RENDER(name) void name (thread_context *Thread, game_memory *Memory,\
												game_input *Input, game_offscreen_buffer *Buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

// NOTE at the moment this has to be a fase function, it cannot be more than a millisecond or so.

#define GAME_GET_SOUND_SAMPLES(name) void name (thread_context *Thread, game_memory *Memory,\
												game_sound_output_buffer *SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

struct game_state {
	int ToneHz;
	int GreenOffset;
	int BlueOffset;

	real32 tSine;

	int PlayerX;
	int PlayerY;
	real32 tJump;
};

#define HANDMADE_H
#endif
