/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  12/8/2020 2:17:50 AM                                          |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright � All rights reserved |======+  */

#if !defined(HANDMADE_SIM_REGION_H)
#define HIT_POINT_SUB_COUNT 4

struct move_spec
{
    bool32 UnitMaxAccelVector;
    f32 Speed;
    f32 Drag;
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

struct hit_point
{
    // TODO(Abid): Bake this down into one variable
    uint8 Flags;
    uint8 FilledAmount;
};

// TODO(Abid): Rename sim_entity to entity!
struct sim_entity;
union entity_reference
{
    sim_entity *Ptr;
    uint32 Index;
};

enum sim_entity_flags
{
    // TODO(Abid): Does it make more sense to have the flag be for _non_ colliding entities?
    // TODO(Abid): Collides and ZSupported probably can be removed now/soon
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

    // TODO(Abid): VolumeCount is always expected to be greater than 0 if the entity
    // has any volume... in the future, this could be compressed if necessary to say
    // that the VolumeCount can be 0 if the TotalVolume should be used as the only
    // collision volume for the entity.
    uint32 VolumeCount;
    sim_entity_collision_volume *Volumes;
};

struct sim_entity
{
    // NOTE(Abid): These are only for the sim region
    world_chunk *OldChunk;
    uint32 StorageIndex;
    bool32 Updatable;

    //
    
    entity_type Type;
    uint32 Flags;
    
    v3 P;
    v3 dP;
    
    f32 DistanceLimit;

    sim_entity_collision_volume_group *Collision;

    f32 FacingDirection;
    f32 tBob;

    int32 dAbsTileZ;

    // TODO(Abid): Should hitpoints themselves be entities?
    uint32 HitPointMax;
    hit_point HitPoint[16];

    entity_reference Sword;

    // TODO(Abid): Only for stairwells!
    v2 WalkableDim;
    f32 WalkableHeight;
    
    // TODO(Abid): Generation index so we know how "up to date" this entity is.
};

struct sim_entity_hash
{
    sim_entity *Ptr;
    uint32 Index;
};

struct sim_region
{
    // TODO(Abid): Need a hash table here to map stored entity indices
    // to sim entities!
    
    world *World;
    f32 MaxEntityRadius;
    f32 MaxEntityVelocity;

    world_position Origin;
    rectangle3 Bounds;
    rectangle3 UpdatableBounds;
    
    uint32 MaxEntityCount;
    uint32 EntityCount;
    sim_entity *Entities;
    
    // TODO(Abid): Do I really want a hash for this??
    // NOTE(Abid): Must be a power of two!
    sim_entity_hash Hash[4096];
};

#define HANDMADE_SIM_REGION_H
#endif
