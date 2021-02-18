/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  12/8/2020 1:31:28 AM                                          |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright © All rights reserved |======+  */

struct world_position
{
	// TODO It seems like we have to store ChunkX/Y/Z with each
	// entity because even though the sim region gather doesn't
	// need it at first, and we could get by without it, entity
	// references pull in entities WITHOUT going through their
	// world_chunk, and thus still need to know the ChunkX/Y/Z

	int32_t ChunkY;
	int32_t ChunkX;
	int32_t ChunkZ;

	// NOTE(Khisrow): These are the offsets from the chunk center
	v3 Offset_;
};

struct world_entity_block
{
	uint32_t EntityCount;
	uint32_t LowEntityIndex[16];
	world_entity_block *Next;
};

struct world_chunk
{
	int32_t ChunkX;
	int32_t ChunkY;
	int32_t ChunkZ;

	// TODO Profile this and determine if a pointer would be better here!
	world_entity_block FirstBlock;

	world_chunk *NextInHash;
};

struct world
{
	// real32 TileSideInMeters;
	// real32 TileDepthInMeters;
	v3 ChunkDimInMeters;

	world_entity_block *FirstFree;

	world_chunk ChunkHash[4096];
};
