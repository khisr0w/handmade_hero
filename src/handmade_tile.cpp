inline void
RecanonicalizeCoord(tile_map *TileMap, uint32_t *Tile, real32 *TileRel)
{
	// NOTE World is toroidal topology, you go from one place and come to another place.
	int32_t Offset = RoundReal32ToInt32(*TileRel / TileMap->TileSideInMeters);
	*Tile += Offset;
	*TileRel -= Offset * TileMap->TileSideInMeters;

	Assert(*TileRel >= -0.5f*TileMap->TileSideInMeters);
	// TODO Fix the floating point math so this can be < only
	Assert(*TileRel <= 0.5f*TileMap->TileSideInMeters);
}

inline tile_map_position
RecanonicalizePosition(tile_map *TileMap, tile_map_position Pos)
{
	tile_map_position Result = Pos;

	RecanonicalizeCoord(TileMap, &Result.AbsTileX, &Result.TileRelX);
	RecanonicalizeCoord(TileMap, &Result.AbsTileY, &Result.TileRelY);
	
	return Result;
}

inline tile_chunk *
GetTileChunk(tile_map *TileMap, uint32_t TileChunkX, uint32_t TileChunkY)
{
	tile_chunk *TileChunk = 0;

	if((TileChunkX >= 0) && (TileChunkX < TileMap->TileChunkCountX) &&
	   (TileChunkY >= 0) && (TileChunkY < TileMap->TileChunkCountY))
	{
		TileChunk = &TileMap->TileChunks[TileChunkY * TileMap->TileChunkCountX + TileChunkX];
	}
	return TileChunk;
}

inline tile_chunk_position
GetChunkPositionFor(tile_map *TileMap, uint32_t AbsTileX, uint32_t AbsTileY)
{
	tile_chunk_position Result;

	Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
	Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
	Result.RelTileX = AbsTileX & TileMap->ChunkMask;
	Result.RelTileY = AbsTileY & TileMap->ChunkMask;

	return Result;
}


inline uint32_t
GetTileValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, uint32_t TileX, uint32_t TileY)
{
	Assert(TileChunk);
	Assert(TileX < TileMap->ChunkDim);
	Assert(TileY < TileMap->ChunkDim);

	return TileChunk->Tiles[TileY*TileMap->ChunkDim + TileX];
}

inline bool32
GetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32_t TestTileX, uint32_t TestTileY)
{
	uint32_t TileValue = 0;

	if(TileChunk)
	{
		TileValue = GetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
	}
	return TileValue;
}

internal bool32
GetTileValue(tile_map * TileMap, uint32_t AbsTileX, uint32_t AbsTileY)
{
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY);

	tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY);
	return GetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY);
}

internal bool32
IsTileMapPointEmpty(tile_map * TileMap, tile_map_position CanPos)
{
	uint32_t TileValue = GetTileValue(TileMap, CanPos.AbsTileX, CanPos.AbsTileY);
	return (TileValue == 0);
}

inline void
SetTileValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, uint32_t TileX, uint32_t TileY, uint32_t TileValue)
{
	Assert(TileChunk);
	Assert(TileX < TileMap->ChunkDim);
	Assert(TileY < TileMap->ChunkDim);

	TileChunk->Tiles[TileY*TileMap->ChunkDim + TileX] = TileValue;
}

inline void
SetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32_t TestTileX, uint32_t TestTileY, uint32_t TileValue)
{
	if(TileChunk)
	{
		SetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY, TileValue);
	}
}

internal void
SetTileValue(memory_arena *Arena, tile_map *TileMap, uint32_t AbsTileX, uint32_t AbsTileY, uint32_t TileValue)
{
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY);

	tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY);

	Assert(TileChunk);
	if(TileChunk)
	{
		SetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY, TileValue);
	}
}
