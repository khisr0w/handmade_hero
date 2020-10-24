#include "handmade.h"
#include "handmade_instrinsics.h"

internal void GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
	int16_t ToneVolume = 3000;
	int WavePeriod =SoundBuffer->SamplesPerSecond/ToneHz;

	int16_t *SampleOut = SoundBuffer->Samples;

	for (int SampleIndex = 0; 
			SampleIndex < SoundBuffer->SampleCountToOutput; 
			++SampleIndex) {
#if 0
		real32 SineValue = sinf(GameState->tSine);
		int16_t SampleValue = (int16_t)(SineValue * ToneVolume); 
#else
		int16_t SampleValue = 0;
#endif
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;
#if 0
		GameState->tSine += 2.0f*PI32 * 1.0f / (real32)WavePeriod;
		if(GameState->tSine > 2.0f*PI32)
		{
			GameState->tSine -= 2.0f*PI32;
		}
#endif
	}
}

internal void
DrawRect(game_offscreen_buffer *Buffer,
		 real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY,
		 real32 R, real32 G, real32 B)
{
	int32_t MinX = RoundReal32ToInt32(RealMinX);
	int32_t MinY = RoundReal32ToInt32(RealMinY);
	int32_t MaxX = RoundReal32ToInt32(RealMaxX);
	int32_t MaxY = RoundReal32ToInt32(RealMaxY);

	if(MinX < 0)
	{
		MinX = 0;
	}
	if(MinY < 0)
	{
		MinY = 0;
	}
	if(MaxX > Buffer->Width)
	{
		MaxX = Buffer->Width;
	}
	if(MaxY > Buffer->Height)
	{
		MaxY = Buffer->Height;
	}

	uint32_t Color = (uint32_t)((RoundReal32ToUInt32(R * 255.0f) << 16) |
								(RoundReal32ToUInt32(G * 255.0f) << 8) |
								(RoundReal32ToUInt32(B * 255.0f) << 0));
	uint8_t *Row = ((uint8_t *)Buffer->Memory +
							   MinX*Buffer->BytesPerPixel +
							   MinY*Buffer->Pitch);

	for (int Y = MinY;
		 Y < MaxY;
		 ++Y)
	{
		uint32_t *Pixel = (uint32_t *)Row;
		for (int X = MinX;
			 X < MaxX;
			 ++X)
		{
			*Pixel++ = Color;
		}	
		Row += Buffer->Pitch;
	}
}

inline tile_chunk *
GetTileChunk(world *World, int32_t TileChunkX, int32_t TileChunkY)
{
	tile_chunk *TileChunk = 0;

	if((TileChunkX >= 0) && (TileChunkX < World->TileChunkCountX) &&
	   (TileChunkY >= 0) && (TileChunkY < World->TileChunkCountY))
	{
		TileChunk = &World->TileChunks[TileChunkY * World->TileChunkCountX + TileChunkX];
	}
	return TileChunk;
}

inline uint32_t
GetTileValueUnchecked(world *World, tile_chunk *TileChunk, uint32_t TileX, uint32_t TileY)
{
	Assert(TileChunk);
	Assert(TileX < World->ChunkDim);
	Assert(TileY < World->ChunkDim);

	return TileChunk->Tiles[TileY*World->ChunkDim + TileX];
}

internal bool32
GetTileValue(world *World, tile_chunk *TileChunk, uint32_t TestTileX, uint32_t TestTileY)
{
	uint32_t TileValue = 0;

	if(TileChunk)
	{
		TileValue = GetTileValueUnchecked(World, TileChunk, TestTileX, TestTileY);
	}
	return TileValue;
}

inline void
RecanonicalizeCoord(world *World, uint32_t *Tile, real32 *TileRel)
{
	// NOTE World is toroidal topology, you go from one place and come to another place.
	int32_t Offset = FloorReal32ToUInt32(*TileRel / World->TileSideInMeters);
	*Tile += Offset;
	*TileRel -= Offset * World->TileSideInMeters;

	Assert(*TileRel >= 0);
	// TODO Fix the floating point math so this can be < only
	Assert(*TileRel <= World->TileSideInMeters);
}

inline world_position
RecanonicalizePosition(world *World, world_position Pos)
{
	world_position Result = Pos;

	RecanonicalizeCoord(World, &Result.AbsTileX, &Result.TileRelX);
	RecanonicalizeCoord(World, &Result.AbsTileY, &Result.TileRelY);
	
	return Result;
}

inline tile_chunk_position
GetChunkPositionFor(world *World, uint32_t AbsTileX, uint32_t AbsTileY)
{
	tile_chunk_position Result;

	Result.TileChunkX = AbsTileX >> World->ChunkShift;
	Result.TileChunkY = AbsTileY >> World->ChunkShift;
	Result.RelTileX = AbsTileX & World->ChunkMask;
	Result.RelTileY = AbsTileY & World->ChunkMask;

	return Result;
}

internal bool32
GetTileValue(world * World, uint32_t AbsTileX, uint32_t AbsTileY)
{
	tile_chunk_position ChunkPos = GetChunkPositionFor(World, AbsTileX, AbsTileY);

	// WARNING Not the same as casey
	tile_chunk *TileChunk = GetTileChunk(World, ChunkPos.TileChunkX, ChunkPos.TileChunkY);
	// WARNING Not the same as casey
	return GetTileValue(World, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY);
}

internal bool32
IsWorldPointEmpty(world * World, world_position CanPos)
{
	uint32_t TileValue = GetTileValue(World, CanPos.AbsTileX, CanPos.AbsTileY);
	return (TileValue == 0);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	Assert((&Input->Controllers[0].Back - &Input->Controllers[0].Buttons[0]) ==
			(ArrayCount(Input->Controllers[0].Buttons) - 1));
	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

#define TILE_MAP_COUNT_X 256
#define TILE_MAP_COUNT_Y 256

	uint32_t TempTiles[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,  1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1,  1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1,  1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1,  1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
		{1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,  1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 0, 1,  1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0, 1,  1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1}
	};

	world World;
	// NOTE this is set to using 256x256 tile chunks.
	World.ChunkShift = 8;
	World.ChunkMask = (1 << World.ChunkShift) - 1;
	World.ChunkDim = 256;

	World.TileChunkCountX = 1;
	World.TileChunkCountY = 1;

	tile_chunk TileChunk;
	TileChunk.Tiles = (uint32_t *)TempTiles;
	World.TileChunks = &TileChunk;

	World.TileSideInMeters = 1.4f;
	World.TileSideInPixels = 60;
	World.MetersToPixels = (real32)World.TileSideInPixels / (real32)World.TileSideInMeters;

	real32 PlayerHeight = 1.4f;
	real32 PlayerWidth = 0.75f * PlayerHeight;

	real32 LowerLeftX = -(real32)World.TileSideInPixels / 2;
	real32 LowerLeftY = (real32)Buffer->Height;

	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		Memory->IsInitialized = true;
		GameState->PlayerP.AbsTileX = 3;
		GameState->PlayerP.AbsTileY = 3;
		GameState->PlayerP.TileRelX = 65.0f / World.MetersToPixels;
		GameState->PlayerP.TileRelY = 65.0f / World.MetersToPixels;
	}

	for (int ControllerIndex = 0;
		 ControllerIndex < ArrayCount(Input->Controllers);
		 ++ControllerIndex)
	{
		game_controller_input *Controller = GetController(Input, ControllerIndex);
		if(Controller->IsAnalog)
		{
			// Do the analogue tunning
		} 
		else
		{
			// Digital input tunning
			real32 dPlayerX = 0.0f; // pixels/second 
			real32 dPlayerY = 0.0f; // pixels/second
			if(Controller->MoveUp.EndedDown)
			{
				dPlayerY = 1.0f;
			}
			if(Controller->MoveDown.EndedDown)
			{
				dPlayerY = -1.0f;
			}
			if(Controller->MoveLeft.EndedDown)
			{
				dPlayerX = -1.0f;
			}
			if(Controller->MoveRight.EndedDown)
			{
				dPlayerX = 1.0f;
			}
			dPlayerX *= 4.0f;
			dPlayerY *= 4.0f;

			// TODO Diagonal will be faster! Fix once we have vectors
			world_position NewPlayerP = GameState->PlayerP;
			NewPlayerP.TileRelX += dPlayerX * Input->dtForFrame;
			NewPlayerP.TileRelY += dPlayerY * Input->dtForFrame;
			NewPlayerP = RecanonicalizePosition(&World, NewPlayerP);

			world_position PlayerLeft = NewPlayerP;
			PlayerLeft.TileRelX -= 0.5f*PlayerWidth;
			PlayerLeft = RecanonicalizePosition(&World, PlayerLeft);

			world_position PlayerRight = NewPlayerP;
			PlayerRight.TileRelX += 0.5f*PlayerWidth;
			PlayerRight = RecanonicalizePosition(&World, PlayerRight);

			if (IsWorldPointEmpty(&World, NewPlayerP) &&
				IsWorldPointEmpty(&World, PlayerLeft) &&
				IsWorldPointEmpty(&World, PlayerRight))
			{
				GameState->PlayerP = NewPlayerP;
			}
		}
	}

	DrawRect(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height, 1.0f, 0.5f, 0.0f);
	
	real32 CenterX = 0.5f * (real32)Buffer->Width;
	real32 CenterY = 0.5f * (real32)Buffer->Height;

	for(int32_t RelRow = -10;
		RelRow < 10;
		++RelRow)
	{
		for(int32_t RelColumn = -20;
			RelColumn < 20;
			++RelColumn)
		{
			uint32_t Column = GameState->PlayerP.AbsTileX + RelColumn;
			uint32_t Row = GameState->PlayerP.AbsTileY + RelRow;
			uint32_t TileID = GetTileValue(&World, Column, Row);
			real32 Gray = 0.5f;
			if(TileID == 1)
			{
				Gray = 1.0f;
			}

			if((Column == GameState->PlayerP.AbsTileX) &&
			   (Row == GameState->PlayerP.AbsTileY))
			{
				Gray = 0.f;
			}

			real32 MinX = CenterX + ((real32)RelColumn) * World.TileSideInPixels;
			real32 MinY = CenterY - ((real32)RelRow) * World.TileSideInPixels;
			real32 MaxX = MinX + World.TileSideInPixels;
			real32 MaxY = MinY - World.TileSideInPixels;
			DrawRect(Buffer, MinX, MaxY, MaxX, MinY, Gray, Gray, Gray);
		}
	}

	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;
	real32 PlayerLeft = CenterX + World.MetersToPixels*GameState->PlayerP.TileRelX - 0.5f*World.MetersToPixels*PlayerWidth;
	real32 PlayerTop = CenterY - World.MetersToPixels*GameState->PlayerP.TileRelY - World.MetersToPixels*PlayerHeight;
	DrawRect(Buffer, 
			 PlayerLeft, PlayerTop,
			 PlayerLeft + PlayerWidth * World.MetersToPixels,
			 PlayerTop + PlayerHeight*World.MetersToPixels,
			 PlayerR, PlayerG, PlayerB);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, 400);
}

/*
internal void RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset) {

	// TODO let's what the optimizer does

	uint8_t *Row = (uint8_t *)Buffer->Memory;
	for(int Y = 0; Y < Buffer->Height; ++Y) {

		uint32_t *Pixel = (uint32_t *)Row;
		for(int X = 0; X < Buffer->Width; ++X) {

			uint8_t Blue = (uint8_t)(X + XOffset);
			uint8_t Green = (uint8_t)(Y + YOffset);
			// Coloring scheme is BGR because fuck windows
			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer->Pitch;
	}
}
*/
