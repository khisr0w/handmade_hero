/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  11/27/2020 5:11:58 AM                                         |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright © All rights reserved |======+  */

#if !defined(HANDMADE_PLATFORM_H)

/*
	HANDMADE_INTERNAL:
	0 - Build for the public release
	1 - Build for developer only

	HANDMADE_SLOW:
	0 - No Slow Code allowed
	1 - Slow code allowed
*/

#ifdef __cplusplus
extern "C" {
#endif

//
//
//

#ifndef COMPILER_MSVC
#define COMPILER_MSVC 0
#endif

#ifndef COMPILER_LLVM
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
// TODO add more compiler switches
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#endif

//
// NOTE Types
//

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>

typedef int32_t bool32;
typedef char bool8;
typedef float real32;
typedef double real64;

typedef size_t memory_index;

#define REAL32MAXIMUM FLT_MAX

#define local_persist static 
#define global_var static
#define internal static
#define PI32 3.14159265359f

#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#if HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int*)0 = 0;}
#else
#define Assert(Expression)
#endif

inline uint32_t SafeTruncateUInt64 (uint64_t Value)
{
	Assert(Value <= 0xFFFFFFFF);
	return (uint32_t)Value;
}
typedef struct thread_context
{
	int Placeholder;
} thread_context;

#if HANDMADE_INTERNAL
/*
   NOTE These are not for doing anything in the shipping game - they are blocking and the write doesn't
   protect against lost data!
*/

enum 
{
	/* 0 */ DebugCycleCounter_GameUpdateAndRender,
	/* 1 */ DebugCycleCounter_RenderGroupToOutput,
	/* 2 */ DebugCycleCounter_DrawRectangleSlowly,
	/* 3 */ DebugCycleCounter_ProcessPixel,
	/* 4 */ DebugCycleCounter_DrawRectangleQuickly,
	DebugCycleCounter_Count
};

typedef struct debug_cycle_counter
{
	uint64_t CycleCount;
	uint32_t HitCount;
} debug_cycle_counter;

extern struct game_memory *GlobalDebugMemory;

typedef struct debug_read_file_result
{
	uint32_t ContentsSize;
	void* Contents;
} debug_read_file_result;
#endif

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, \
	   																	  char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name (thread_context *Thread, char *Filename, \
															uint32_t MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

typedef struct game_sound_output_buffer
{
	int SampleCountToOutput;
	int16_t *Samples;
	int SamplesPerSecond; 
} game_sound_output_buffer;

#define BITMAP_BYTES_PER_PIXEL 4
typedef struct game_offscreen_buffer
{
	// NOTE Pixels are always 32-bit wide, Memory order BB GG RR XX
	void *Memory;
	int Width; 
	int Height;
	int Pitch;
} game_offscreen_buffer;

typedef struct game_button_state
{
	int HalfTransitionCount;
	bool32 EndedDown;
} game_button_state;

typedef struct game_controller_input
{
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
} game_controller_input;

typedef struct game_input 
{
	game_button_state MouseButtons[5];
	int32_t MouseX, MouseY, MouseZ;

	bool32 ExecutableReloaded;
	real32 dtForFrame;

	game_controller_input Controllers[5];
} game_input;

inline game_controller_input *GetController (game_input *Input, int unsigned ControllerIndex)
{
	Assert(ControllerIndex < ArrayCount(Input->Controllers));

	game_controller_input *Result = &Input->Controllers[ControllerIndex];
	return Result;
}

typedef struct game_memory
{
	bool32 IsInitialized;

	uint64_t PermanentStorageSize;
	void *PermanentStorage; // This memory is required to be zero at startup

	uint64_t TransientStorageSize;
	void *TransientStorage; // This memory is required to be zero at startup

	debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
	debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
	debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;

#if HANDMADE_INTERNAL
	debug_cycle_counter Counters[DebugCycleCounter_Count];
#endif

#if _MSC_VER
	#define BEGIN_TIMED_BLOCK(ID) uint64_t StarCycleCount##ID = __rdtsc()
	#define END_TIMED_BLOCK(ID) GlobalDebugMemory->Counters[DebugCycleCounter_##ID].CycleCount +=  __rdtsc() - StarCycleCount##ID; ++GlobalDebugMemory->Counters[DebugCycleCounter_##ID].HitCount;
	#define END_TIMED_BLOCK_COUNTED(ID, Count) GlobalDebugMemory->Counters[DebugCycleCounter_##ID].CycleCount +=  __rdtsc() - StarCycleCount##ID; GlobalDebugMemory->Counters[DebugCycleCounter_##ID].HitCount += (Count);
#else
	#define BEGIN_TIMED_BLOCK(ID)
	#define END_TIMED_BLOCK(ID)
#endif
} game_memory;

#define GAME_UPDATE_AND_RENDER(name) void name (thread_context *Thread, game_memory *Memory,\
												game_input *Input, game_offscreen_buffer *Buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

// NOTE at the moment this has to be a fase function, it cannot be more than a millisecond or so.

#define GAME_GET_SOUND_SAMPLES(name) void name (thread_context *Thread, game_memory *Memory,\
												game_sound_output_buffer *SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#ifdef __cplusplus
}
#endif

#define HANDMADE_PLATFORM_H
#endif
