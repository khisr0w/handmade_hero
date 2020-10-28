// This is the header file for the platform independent layer

#include "handmade_platform.h"

#define local_persist static 
#define global_var static
#define internal static
#define PI32 3.14159265359f

// TODO should this always be 64-bit?
#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#if !defined(HANDMADE_H)
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

/* 
   TODO Services that the platform layer provides to the game
*/


/* 
   Services that the game provide to the platform layer
*/

// FOUR THINGS - Timing, Controller/Keyboard, Bitmap buffer to use, Sound Buffer to use

inline game_controller_input *GetController (game_input *Input, int unsigned ControllerIndex)
{
	Assert(ControllerIndex < ArrayCount(Input->Controllers));

	game_controller_input *Result = &Input->Controllers[ControllerIndex];
	return Result;
}

//
//
//

struct memory_arena
{
	memory_index Size;
	uint8_t *Base;
	memory_index Used;
};

internal void
InitializeArena(memory_arena *Arena, memory_index Size, uint8_t *Base)
{
	Arena->Size = Size;
	Arena->Base = Base;
	Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, Count*sizeof(type))
void *
PushSize_(memory_arena *Arena, memory_index Size)
{
	Assert((Arena->Used + Size) <= Arena->Size);
	void *Result = Arena->Base + Arena->Used;
	Arena->Used += Size;

	return Result;
}

#include "handmade_instrinsics.h"
#include "handmade_tile.h"

struct world
{
	tile_map *TileMap;
};

struct game_state
{
	memory_arena WorldArena;
	world* World;
	uint32_t *PixelPointer;

	tile_map_position PlayerP;
};

#define HANDMADE_H
#endif
