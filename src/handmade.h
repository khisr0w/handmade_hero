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

struct canonical_position
{
#if 1
	int32_t TileMapX;
	int32_t TileMapY;

	int32_t TileX;
	int32_t TileY;
#else
	uint32_t _TileY;
	uint32_t _TileX;
#endif

	// NOTE This is Tile-relative X and Y
	real32 TileRelX;
   	real32 TileRelY;
};

struct tile_map
{
	uint32_t *Tiles;
};

struct world
{
	real32 TileSideInMeters;
	int32_t TileSideInPixels;
	real32 MetersToPixels;

	int32_t CountX;
	int32_t CountY;

	real32 UpperLeftX;
	real32 UpperLeftY; 
	int32_t TileMapCountX;
	int32_t TileMapCountY;

	tile_map *TileMaps;
};

struct game_state
{
	canonical_position PlayerP;
};

#define HANDMADE_H
#endif
