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

struct tile_chunk_position
{
	uint32_t TileChunkX;
	uint32_t TileChunkY;

	uint32_t RelTileX;
	uint32_t RelTileY;
};

struct world_position
{
	uint32_t AbsTileY;
	uint32_t AbsTileX;

	// NOTE This is Tile-relative X and Y
	// TODO rename to Offset
	real32 TileRelX;
   	real32 TileRelY;
};

struct tile_chunk
{
	uint32_t *Tiles;
};

struct world
{
	uint32_t ChunkShift;
	uint32_t ChunkMask;
	uint32_t ChunkDim;

	real32 TileSideInMeters;
	int32_t TileSideInPixels;
	real32 MetersToPixels;

	int32_t TileChunkCountX;
	int32_t TileChunkCountY;

	tile_chunk *TileChunks;
};

struct game_state
{
	world_position PlayerP;
};

#define HANDMADE_H
#endif
