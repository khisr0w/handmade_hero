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

inline uint32_t SafeTruncateUInt64 (uint64_t Value) {

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

struct tile_map
{
	int32_t CountX;
	int32_t CountY;

	real32 UpperLeftX;
	real32 UpperLeftY; 
	real32 TileHeight; 
	real32 TileWidth;

	uint32_t *Tiles;
};

struct world
{
	int32_t TileMapCountX;
	int32_t TileMapCountY;

	tile_map *TileMaps;
};
struct game_state
{
	real32 PlayerX = 10;
	real32 PlayerY = 10;
};

#define HANDMADE_H
#endif
