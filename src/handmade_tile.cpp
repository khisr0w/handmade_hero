inline tile_chunk *
GetTileChunk(tile_map *TileMap, uint32_t TileChunkX, uint32_t TileChunkY, uint32_t TileChunkZ)
{
	tile_chunk *TileChunk = 0;

	if((TileChunkX >= 0) && (TileChunkX < TileMap->TileChunkCountX) &&
	   (TileChunkY >= 0) && (TileChunkY < TileMap->TileChunkCountY) &&
	   (TileChunkZ >= 0) && (TileChunkZ < TileMap->TileChunkCountZ))
	{
		TileChunk = &TileMap->TileChunks[TileChunkZ*TileMap->TileChunkCountX*TileMap->TileChunkCountY + 
										 TileChunkY * TileMap->TileChunkCountX +
										 TileChunkX];
	}
	return TileChunk;
}

inline tile_chunk_position
GetChunkPositionFor(tile_map *TileMap, uint32_t AbsTileX, uint32_t AbsTileY, uint32_t AbsTileZ)
{
	tile_chunk_position Result;

	Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
	Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
	Result.TileChunkZ = AbsTileZ;
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

	if(TileChunk && TileChunk->Tiles)
	{
		TileValue = GetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
	}
	return TileValue;
}

inline bool32
GetTileValue(tile_map * TileMap, uint32_t AbsTileX, uint32_t AbsTileY, uint32_t AbsTileZ)
{
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);

	tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
	return GetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY);
}

internal bool32
IsTileValueEmpty(uint32_t TileValue)
{
	return ((TileValue == 1) ||
			(TileValue == 3) ||
			(TileValue == 4));
}

inline bool32
GetTileValue(tile_map * TileMap, tile_map_position Pos)
{
	return GetTileValue(TileMap, Pos.AbsTileX, Pos.AbsTileY, Pos.AbsTileZ);
}

internal bool32
IsTileMapPointEmpty(tile_map * TileMap, tile_map_position CanPos)
{
	uint32_t TileValue = GetTileValue(TileMap, CanPos);
	return IsTileValueEmpty(TileValue);
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
	if(TileChunk && TileChunk->Tiles)
	{
		SetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY, TileValue);
	}
}

internal void
SetTileValue(memory_arena *Arena, tile_map *TileMap,
			 uint32_t AbsTileX, uint32_t AbsTileY, uint32_t AbsTileZ,
			 uint32_t TileValue)
{
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);

	tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);

	Assert(TileChunk);
	if(!TileChunk->Tiles)
	{
		uint32_t TileCount = TileMap->ChunkDim*TileMap->ChunkDim;
		TileChunk->Tiles = PushArray(Arena, TileCount, uint32_t);
		for(uint32_t TileIndex = 0;
			TileIndex < TileCount;
			++TileIndex)
		{
			TileChunk->Tiles[TileIndex] = 1;
		}
	}
	SetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY, TileValue);
}

// TODO Should they be in some other file, geometry?
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

	RecanonicalizeCoord(TileMap, &Result.AbsTileX, &Result.Offset.X);
	RecanonicalizeCoord(TileMap, &Result.AbsTileY, &Result.Offset.Y);
	
	return Result;
}

inline bool32
AreOnSameTile(tile_map_position *A, tile_map_position *B)
{
	return ((A->AbsTileX == B->AbsTileX) &&
			(A->AbsTileY == B->AbsTileY) &&
			(A->AbsTileZ == B->AbsTileZ));
}

inline tile_map_difference
Subtract(tile_map *TileMap, tile_map_position *A, tile_map_position *B)
{
	tile_map_difference Result;

	v2 dTileXY = {(real32)A->AbsTileX - (real32)B->AbsTileX,
				  (real32)A->AbsTileY - (real32)B->AbsTileY};
	real32 dTileZ = (real32)A->AbsTileZ - (real32)B->AbsTileZ;

	Result.dXY = TileMap->TileSideInMeters*dTileXY + (A->Offset - B->Offset);

	Result.dZ = TileMap->TileSideInMeters*dTileZ;

	return Result;
}

internal tile_map_position
CenteredTilePoint(uint32_t AbsTileX, uint32_t AbsTileY, uint32_t AbsTileZ)
{
	tile_map_position Result = {};

	Result.AbsTileX = AbsTileX;
	Result.AbsTileY = AbsTileY;
	Result.AbsTileZ = AbsTileZ;

	return Result;
}
