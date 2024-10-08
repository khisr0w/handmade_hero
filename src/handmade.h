/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  12/11/2020 5:43:37 PM                                         |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright � All rights reserved |======+  */

/*
	TODO(Abid):

	- Asset streaming
      - File format
      - Memory management

	- Audio
	  - Sound effect triggers
	  - Ambient sounds
	  - Music

	- Rendering
	  - Straighten out all coordinate systems
	    - Screen
	    - World
	    - Texture
	  - Particle Systems
	  - Lighting
	  - Final Optimization

	- Debug code
  	  - Fonts
  	  - Logging
	  - Diagramming
	  - (A LITTLE GUI, but only a little!) Switches / sliders / etc.
	  - Thread Visualization

	- Metagame / save game?
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
	-> GAME
	  - Entity system
	  - World generation
*/

#if !defined(HANDMADE_H)

#include "handmade_platform.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

//
//
//

struct memory_arena {
    memory_index Size;
    uint8 *Base;
    memory_index Used;

    int32 TempCount;
};

struct temporary_memory {
    memory_arena *Arena;
    memory_index Used;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base) {
    Arena->Size = Size;
    Arena->Base = (uint8 *)Base;
    Arena->Used = 0;
    Arena->TempCount = 0;
}

inline memory_index
GetAligmentOffset(memory_arena *Arena, memory_index Alignment) {
	memory_index AlignmentOffset = 0;
	memory_index ResultPointer = (memory_index)Arena->Base + Arena->Used;

	memory_index AlignmentMask = Alignment - 1;
	if(ResultPointer & AlignmentMask) AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);

	return AlignmentOffset;
}

inline memory_index
GetArenaSizeRemaining(memory_arena *Arena, memory_index Alignment = 4) {
	memory_index Result = Arena->Size - (Arena->Used + GetAligmentOffset(Arena, Alignment));

	return Result;
}

#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ## __VA_ARGS__)
#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ## __VA_ARGS__)
inline void *
PushSize_(memory_arena *Arena, memory_index Size, memory_index Alignment = 4) {
	memory_index AlignmentOffset = GetAligmentOffset(Arena, Alignment);
	Size += AlignmentOffset;

    Assert((Arena->Used + Size) <= Arena->Size);
	void *Result = Arena->Base + Arena->Used + AlignmentOffset;
    Arena->Used += Size;
    
    return Result;
}

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena) {
    temporary_memory Result;

    Result.Arena = Arena;
    Result.Used = Arena->Used;

    ++Arena->TempCount;

    return Result;
}

inline void
EndTemporaryMemory(temporary_memory TempMem) {
    memory_arena *Arena = TempMem.Arena;
    Assert(Arena->Used >= TempMem.Used);
    Arena->Used = TempMem.Used;
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void
CheckArena(memory_arena *Arena) {
    Assert(Arena->TempCount == 0);
}

inline void
SubArena(memory_arena *Result, memory_arena *Arena, memory_index Size, memory_index Alignment = 16) {
	Result->Size = Size;
	Result->Base = (uint8 *)PushSize_(Arena, Size, Alignment);
	Result->Used = 0;
	Result->TempCount = 0;
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
inline void
ZeroSize(memory_index Size, void *Ptr) {
    // TODO(Abid): Check this guy for performance
    uint8 *Byte = (uint8 *)Ptr;
    while(Size--) *Byte++ = 0;
}

#include "handmade_instrinsics.h"
#include "handmade_math.h"
#include "handmade_world.h"
#include "handmade_sim_region.h"
#include "handmade_entity.h"
#include "handmade_render_group.h"
#include "handmade_asset.h"

struct low_entity {
    // TODO(Abid): It's kind of busted that P's can be invalid here,
    // AND we store whether they would be invalid in the flags field...
    // Can we do something better here?
    world_position P;
    sim_entity Sim;
};

struct controlled_hero {
    uint32 EntityIndex;
    
    // NOTE(Abid): These are the controller requests for simulation
    v2 ddP;
    v2 dSword;
    real32 dZ;
};

struct pairwise_collision_rule {
    bool32 CanCollide;
    uint32 StorageIndexA;
    uint32 StorageIndexB;
    
    pairwise_collision_rule *NextInHash;
};
struct game_state;
internal void AddCollisionRule(game_state *GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 ShouldCollide);
internal void ClearCollisionRulesFor(game_state *GameState, uint32 StorageIndex);

struct ground_buffer {
    // NOTE(Abid): An invalid P tells us that this ground_buffer has not been filled
    world_position P; // NOTE(Abid): This is the center of the bitmap
    loaded_bitmap Bitmap;
};

struct hero_bitmap_ids {
    bitmap_id Head;
    bitmap_id Cape;
    bitmap_id Torso;
};


struct game_state {
    bool32 IsInitialized;

    memory_arena WorldArena;
    world *World;

    real32 TypicalFloorHeight;
    
    // TODO(Abid): Should we allow split-screen?
    uint32 CameraFollowingEntityIndex;
    world_position CameraP;

    controlled_hero ControlledHeroes[ArrayCount(((game_input *)0)->Controllers)];

    // TODO(Abid): Change the name to "stored entity"
    uint32 LowEntityCount;
    low_entity LowEntities[100000];

    // TODO(Abid): Must be power of two
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

    real32 Time;

    loaded_bitmap TestDiffuse; // TODO(Abid): Re-fill this thing with gray.
    loaded_bitmap TestNormal;
};

struct task_with_memory {
	bool32 BeingUsed;
	memory_arena Arena;

	temporary_memory MemoryFlush;
};

struct transient_state {
    bool32 IsInitialized;
    memory_arena TranArena;    

	task_with_memory Tasks[4];

	game_assets *Assets;

    uint32 GroundBufferCount;
    ground_buffer *GroundBuffers;
	platform_work_queue *HighPriorityQueue;
	platform_work_queue *LowPriorityQueue;

    uint32 EnvMapWidth;
    uint32 EnvMapHeight;
    // NOTE(Abid): 0 is bottom, 1 is middle, 2 is top
    environment_map EnvMaps[3];
};

inline low_entity *
GetLowEntity(game_state *GameState, uint32 Index) {
    low_entity *Result = 0;
    
    if((Index > 0) && (Index < GameState->LowEntityCount))
    {
        Result = GameState->LowEntities + Index;
    }

    return Result;
}

global_var platform_add_entry *PlatformAddEntry;
global_var platform_complete_all_work *PlatformCompleteAllWork;
global_var debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;

internal task_with_memory *BeginTaskWithMemory(transient_state *TranState);
internal void EndTaskWithMemory(task_with_memory *Task);

#define HANDMADE_H
#endif
