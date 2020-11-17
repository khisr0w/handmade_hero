// This is the header file for the platform independent layer
#if !defined(HANDMADE_H)

#include "handmade_platform.h"

#define Minimum(A, B) (((A) < (B)) ? A : B)
#define Maximum(A, B) (((A) > (B)) ? A : B)

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

#include "handmade_math.h"
#include "handmade_instrinsics.h"
#include "handmade_world.h"

struct loaded_bitmap
{
	int32_t Width;
	int32_t Height;
	uint32_t *Pixels;
};

struct hero_bitmaps
{
	v2 Align;

	loaded_bitmap Head;
	loaded_bitmap Cape;
	loaded_bitmap Torso;
};

enum entity_type
{
	EntityType_Null,
	EntityType_Hero,
	EntityType_Wall,
	EntityType_Monster,
	EntityType_Familiar,
};

struct high_entity
{
	v2 P;
	v2 dP;
	uint32_t ChunkZ;
	uint32_t FacingDirection;

	real32 tBob;

	real32 dZ;
	real32 Z;

	uint32_t LowEntityIndex;
};

struct low_entity
{
	world_position P;
	real32 Height, Width;
	entity_type Type;

	bool32 Collides;
	int32_t dAbsTileZ;

	uint32_t HighEntityIndex;
};

struct entity
{
	uint32_t LowIndex;
	low_entity *Low;
	high_entity *High;
};

struct entity_visible_piece
{
	loaded_bitmap *Bitmap;
	v2 Offset;
	real32 OffsetZ;
	real32 Alpha;
};

struct entity_visible_piece_group
{
	uint32_t PieceCount;
	entity_visible_piece Pieces[8];
};

struct game_state
{
	memory_arena WorldArena;
	world* World;

	// TODO Split-screen?
	uint32_t CameraFollowingEntityIndex;
	world_position CameraP;

	uint32_t PlayerIndexForController[ArrayCount(((game_input *)0)->Controllers)];

	uint32_t LowEntityCount;
	low_entity LowEntities[100000];

	uint32_t HighEntityCount;
	high_entity HighEntities_[256];

	loaded_bitmap BackDrop;
	loaded_bitmap Shadow;
	hero_bitmaps HeroBitmaps[4];

	loaded_bitmap Tree;
};

#define HANDMADE_H
#endif
