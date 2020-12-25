/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  12/8/2020 2:17:50 AM                                          |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright © All rights reserved |======+  */

#if !defined(HANDMADE_SIM_REGION_H)
#define HIT_POINT_SUB_COUNT 4

struct move_spec
{
	bool32 UnitMaxAccelVector;
	real32 Speed;
	real32 Drag;
};

struct hit_point
{
	// TODO Bake this down into one variable
	uint8_t Flags;
	uint8_t FilledAmount;
};

enum entity_type
{
	EntityType_Null,

	EntityType_Space,

	EntityType_Hero,
	EntityType_Wall,
	EntityType_Familiar,
	EntityType_Monstar,
	EntityType_Sword,
	EntityType_Stairwell,
};

struct sim_entity;
union entity_reference
{
	sim_entity *Ptr;
	uint32_t Index;
};

enum sim_entity_flags
{
	// TODO Collides and Z-supported can probably be removed now/soon
	EntityFlag_Collides = (1 << 0),
	EntityFlag_Nonspatial = (1 << 1),
	EntityFlag_Moveable = (1 << 2),
	EntityFlag_ZSupported = (1 << 3),
	EntityFlag_Traversable = (1 << 4),

	EntityFlag_Simming = (1 << 30),
};

struct sim_entity_collision_volume
{
	v3 OffsetP;
	v3 Dim;
};

struct sim_entity_collision_volume_group
{
	sim_entity_collision_volume TotalVolume;

	// NOTE Volume count is always expected to be greater than 0 if the entity
	// has any volume... in the future, this could be compressed if necessary
	// to say that the VolumeCount can be 0 if the TotalVolume should be used
	// as the only collision volume for the entity.
	uint32_t VolumeCount;
	sim_entity_collision_volume *Volumes;
};

struct sim_entity
{
	// NOTE These are only for the sim region
	uint32_t StorageIndex;
	bool32 Updatable;

	//

	entity_type Type;
	uint32_t Flags;

	v3 P;
	v3 dP;

	real32 DistanceLimit;

	sim_entity_collision_volume_group *Collision;

	uint32_t FacingDirection;
	real32 tBob;

	int32_t dAbsTileZ;

	// TODO Should hit points themselves be entities?
	uint32_t HitPointMax;
	hit_point HitPoint[16];

	entity_reference Sword;

	// TODO Only for stairwells!
	v2 WalkableDim;
	real32 WalkableHeight;
};

struct sim_entity_hash
{
	sim_entity *Ptr;
	uint32_t Index;
};

struct sim_region
{
	world *World;
	real32 MaxEntityRadius;
	real32 MaxEntityVelocity;

	world_position Origin;
	rectangle3 Bounds;
	rectangle3 UpdatableBounds;

	uint32_t MaxEntityCount;
	uint32_t EntityCount;
	sim_entity *Entities;

	sim_entity_hash Hash[4096];
};
#define HANDMADE_SIM_REGION_H
#endif
