/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  11/27/2020 5:11:58 AM                                         |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright � All rights reserved |======+  */

#if !defined(HANDMADE_PLATFORM_H)

/*
  NOTE(Abid):

  HANDMADE_INTERNAL:
    0 - Build for public release
    1 - Build for developer only

  HANDMADE_SLOW:
    0 - Not slow code allowed!
    1 - Slow code welcome.
*/

#ifdef __cplusplus
extern "C" {
#endif

//
// NOTE(Abid): Compilers
//
    
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif
    
#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
// TODO(Abid): Moar compilerz!!!
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#elif COMPILER_LLVM
#include <x86intrin.h>
#else
#error SEE/NEON optimizations are not available for this compiler yet!!!!
#endif
    
//
// NOTE(Abid): Types
//
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
    
// typedef int8_t i8;
// typedef int16_t i16;
// typedef int32_t i32;
// typedef int64_t i64;
// typedef int32 bool;
// 
// typedef uint8_t u8;
// typedef uint16_t u16;
// typedef uint32_t u32;
// typedef uint64_t u64;
// 
// typedef intptr_t iptr;
// typedef uintptr_t uptr;
// 
// typedef size_t memory_index;
//     
// typedef float f32;
// typedef double f64;

/* TODO(Abid): Remove of this verbose types */
typedef int8_t int8;
typedef int16_t int16;
typedef int16_t i16;
typedef int32_t int32;
typedef int32_t i32;
typedef int64_t int64;
typedef int64_t i64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint8_t u8;
typedef uint16_t uint16;
typedef uint16_t u16;
typedef uint32_t uint32;
typedef uint32_t u32;
typedef uint64_t uint64;
typedef uint64_t u64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

typedef float real32;
typedef float f32;
typedef double real64;
typedef double f64;

#define Real32Maximum FLT_MAX
    
#define internal static
#define local_persist static
#define global_var static

#define Tau32 6.28318530717958647692f
#define Pi32 3.14159265359f

#if HANDMADE_SLOW
// TODO(Abid): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(Abid): swap, min, max ... macros???

#define Align16(Value) ((Value + 15) & ~15)

inline uint32
SafeTruncateUInt64(uint64 Value)
{
    // TODO(Abid): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return Result;
}

typedef struct thread_context
{
    int Placeholder;
} thread_context;

/*
  NOTE(Abid): Services that the platform layer provides to the game
*/
#if HANDMADE_INTERNAL
/* IMPORTANT(Abid):

   These are NOT for doing anything in the shipping game - they are
   blocking and the write doesn't protect against lost data!
*/
typedef struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
} debug_read_file_result;

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(char *Filename, uint32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

enum
{
    /* 0 */ DebugCycleCounter_GameUpdateAndRender,
    /* 1 */ DebugCycleCounter_RenderGroupToOutput,
    /* 2 */ DebugCycleCounter_DrawRectangleSlowly,
    /* 3 */ DebugCycleCounter_ProcessPixel,
    /* 4 */ DebugCycleCounter_DrawRectangleQuickly,
    DebugCycleCounter_Count,
};
typedef struct debug_cycle_counter
{    
    uint64 CycleCount;
    uint32 HitCount;
} debug_cycle_counter;

extern struct game_memory *DebugGlobalMemory;
#if _MSC_VER
#define BEGIN_TIMED_BLOCK(ID) uint64 StartCycleCount##ID = __rdtsc();
#define END_TIMED_BLOCK(ID) DebugGlobalMemory->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID; ++DebugGlobalMemory->Counters[DebugCycleCounter_##ID].HitCount;
// TODO(Abid): Clamp END_TIMED_BLOCK_COUNTED so that if the calc is wrong, it won't overflow!
#define END_TIMED_BLOCK_COUNTED(ID, Count) DebugGlobalMemory->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID; DebugGlobalMemory->Counters[DebugCycleCounter_##ID].HitCount += (Count);
#else
#define BEGIN_TIMED_BLOCK(ID) 
#define END_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK_COUNTED(ID, Count)
#endif
    
#endif

/*
  NOTE(Abid): Services that the game provides to the platform layer.
  (this may expand in the future - sound on separate thread, etc.)
*/

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// TODO(Abid): In the future, rendering _specifically_ will become a three-tiered abstraction!!!
#define BITMAP_BYTES_PER_PIXEL 4
typedef struct game_offscreen_buffer
{
    // NOTE(Abid): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    void *Memory;
    int Width;
    int Height;
    int Pitch;
} game_offscreen_buffer;

typedef struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
} game_sound_output_buffer;

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
    
    union
    {
        game_button_state Buttons[12];
        struct
        {
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

            game_button_state Back;
            game_button_state Start;

            // NOTE(Abid): All buttons must be added above this line
            
            game_button_state Terminator;
        };
    };
} game_controller_input;

typedef struct game_input
{
    game_button_state MouseButtons[5];
    int32 MouseX, MouseY, MouseZ;

    bool32 ExecutableReloaded;
    real32 dtForFrame;

    game_controller_input Controllers[5];
} game_input;

struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_work_queue *Queue, void *Data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

#define PLATFORM_ADD_ENTRY(name) void name(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
typedef PLATFORM_ADD_ENTRY(platform_add_entry);
#define PLATFORM_COMPLETE_ALL_WORK(name) void name(platform_work_queue *Queue)
typedef PLATFORM_COMPLETE_ALL_WORK(platform_complete_all_work);
typedef struct game_memory {
    uint64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(Abid): REQUIRED to be cleared to zero at startup

    uint64 TransientStorageSize;
    void *TransientStorage; // NOTE(Abid): REQUIRED to be cleared to zero at startup

	platform_work_queue *HighPriorityQueue;
	platform_work_queue *LowPriorityQueue;

	platform_add_entry *PlatformAddEntry;
	platform_complete_all_work *PlatformCompleteAllWork;

    debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
    debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
    debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;

#if HANDMADE_INTERNAL
    debug_cycle_counter Counters[DebugCycleCounter_Count];
#endif
} game_memory;

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

// NOTE(Abid): At the moment, this has to be a very fast function, it cannot be
// more than a millisecond or so.
// TODO(Abid): Reduce the pressure on this function's performance by measuring it
// or asking about it, etc.
#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory *Memory, game_sound_output_buffer *SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return Result;
}

#ifdef __cplusplus
}
#endif

#define HANDMADE_PLATFORM_H
#endif
