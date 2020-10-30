// This is the header file for the platform independent layer
#if !defined(HANDMADE_H)

#include "handmade_platform.h"

/* 
   TODO Services that the platform layer provides to the game
*/


/* 
   Services that the game provide to the platform layer
*/

// FOUR THINGS - Timing, Controller/Keyboard, Bitmap buffer to use, Sound Buffer to use

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

struct loaded_bitmap
{
	int32_t Width;
	int32_t Height;
	uint32_t *Pixels;
};

struct hero_bitmaps
{
	int32_t AlignX;
	int32_t AlignY;

	loaded_bitmap Head;
	loaded_bitmap Cape;
	loaded_bitmap Torso;
};

struct game_state
{
	memory_arena WorldArena;
	world* World;

	tile_map_position CameraP;
	tile_map_position PlayerP;

	loaded_bitmap BackDrop;
	uint32_t HeroFacingDirection;
	hero_bitmaps HeroBitmaps[4];
};

#define HANDMADE_H
#endif
