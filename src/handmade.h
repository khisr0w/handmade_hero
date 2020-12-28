/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  12/11/2020 5:43:37 PM                                         |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright � All rights reserved |======+  */

/*
	TODO:

	ARCHITECTURE EXPLORATION
	- Z!
	  - Need to make a solid concept of ground levels to the camera
	  	can be freely placed in Z and have multiple ground levels
		in one sim region.
	  - Concept of ground in the collision loop so it can handle
	  	collisions coming onto AND OFF OF stairwells, for example.
	  - Go through and define how tall everything should be
	  - Make sure flying things can go through low walls
	  - How is this rendered?
	  - ZFudge!!!!!!
	- Collision Detection?
	  - Fix sword collision!
	  - Clean up perdicate proliferation! Can we make a nice clean
	  	set of flags/rules so that it's easy to understand how 
		things work in terms of special handling? This may involve
		making iteration handle everything instead of handling
		overlap outside and so on.
	  - Transient collision rules! Clear based on flag.
	    - Allow non-transient rules to override transient ones.
		- Entry/exit?
	  - What's the plan for robustness / shape definition?
	  - (Implement reprojection to handle interpenetration)
	- Implement multiple sim regions per frame
	  - Per-entity clocking
	  - Sim region merging? For multiple players?
	  - Simple zoom out view for testing
	  - Draw tile chunks so we can verify that things are aligned /
		in the chunks we want them to be in / etc...

	- Debug code
  	  - Logging
	  - Diagramming
	  - (A LITTLE GUI, but only a little!) Switches / sliders / etc.

	- Audio
	  - Sound effect triggers
	  - Ambient sounds
	  - Music

	- Asset streaming

	- Metagame / save game?
	  - How do you enter "save slot"?
	  - Persistent unlocks/etc.
	  - Do we allow saved games? Probably yes, just only for "pausing",
	  * Continuous save for crash recovery?
	- Rudimentary world gen (no quality, just "what sorts of things" we do)
	  - Placement of background things
	  - Connectivity?
	  - Non-overlapping?
	  - Map display
	    - Magnets - how do they work?

	- AI
	  - Rudimentary monstar behanvior example
	  * Pathfinding
	  - AI "storage"

	* Animation, should probably lead into the rendering
	  - Skeletal animation
	  - Particle systems

	PRODUCTION
	- Rendering
	-> GAME
	  - Entity system
	  - World generation
*/

#if !defined(HANDMADE_H)

#include "handmade_platform.h"
#include "handmade_random.h"

#define Minimum(A, B) (((A) < (B)) ? A : B)
#define Maximum(A, B) (((A) > (B)) ? A : B)

/* 
   TODO Services that the platform layer provides to the game
*/


/* 
   Services that the game provide to the platform layer
*/

// FOUR THINGS - Timing, Controller/Keyboard, Bitmap buffer to use, Sound Buffer to use

struct memory_arena
{
	memory_index Size;
	uint8_t *Base;
	memory_index Used;
};

internal void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
	Arena->Size = Size;
	Arena->Base = (uint8_t *)Base;
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

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance));
inline void
ZeroSize(memory_index Size, void *Ptr)
{
	// TODO Check this guy for performance!
	uint8_t *Byte = (uint8_t *)Ptr;
	while(Size--)
	{
		*Byte++ = 0;
	}
}

#include "handmade_instrinsics.h"
#include "handmade_math.h"
#include "handmade_world.h"
#include "handmade_sim_region.h"
#include "handmade_entity.h"

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

struct low_entity
{
	world_position P;
	sim_entity Sim;
};

struct entity_visible_piece
{
	loaded_bitmap *Bitmap;
	v2 Offset;
	real32 OffsetZ;
	real32 EntityZC;

	real32 R, G, B, A;
	v2 Dim;
};

struct controlled_hero
{
	uint32_t EntityIndex;
	// NOTE These are the controller requests for simulation
	v2 ddP;
	v2 dSword;
	real32 dZ;
};

struct pairwise_collision_rule
{
	bool32 CanCollide;
	uint32_t StorageIndexA;
	uint32_t StorageIndexB;

	pairwise_collision_rule *NextInHash;
};
struct game_state;
internal void 
AddCollisionRule(game_state *GameState, uint32_t StorageIndexA, uint32_t StorageIndexB, bool32 CanCollide);
internal void
ClearCollisionRulesFor(game_state *GameState, uint32_t StorageIndex);

struct game_state
{
	memory_arena WorldArena;
	world* World;

	real32 MetersToPixels;

	// TODO Split-screen?
	uint32_t CameraFollowingEntityIndex;
	world_position CameraP;

	controlled_hero ControlledHeroes[ArrayCount(((game_input *)0)->Controllers)];

	uint32_t LowEntityCount;
	low_entity LowEntities[100000];

	loaded_bitmap Grass[2];
	loaded_bitmap Stone[4];
	loaded_bitmap Tuft[3];

	loaded_bitmap BackDrop;
	loaded_bitmap Shadow;
	hero_bitmaps HeroBitmaps[4];

	loaded_bitmap Tree;
	loaded_bitmap Sword;
	loaded_bitmap Stairwell;

	// TODO This must be a power of two
	pairwise_collision_rule *CollisionRuleHash[256];
	pairwise_collision_rule *FirstFreeCollisionRule;

	sim_entity_collision_volume_group *NullCollision;
	sim_entity_collision_volume_group *SwordCollision;
	sim_entity_collision_volume_group *StairCollision;
	sim_entity_collision_volume_group *PlayerCollision;
	sim_entity_collision_volume_group *MonstarCollision;
	sim_entity_collision_volume_group *FamiliarCollision;
	sim_entity_collision_volume_group *WallCollision;
	sim_entity_collision_volume_group *StandardRoomCollision;
};

struct entity_visible_piece_group
{
	game_state *GameState;
	uint32_t PieceCount;
	entity_visible_piece Pieces[8];
};

inline low_entity *
GetLowEntity(game_state *GameState, uint32_t Index)
{
	low_entity *Result = 0;

	if((Index > 0) && (Index < GameState->LowEntityCount))
	{
		Result = GameState->LowEntities + Index;
	}

	return Result;
}

#define HANDMADE_H
#endif
