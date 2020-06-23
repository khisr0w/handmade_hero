#include "handmade.h"

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

inline int32_t
RoundReal32ToInt32(real32 Real32)
{
	return (int32_t)(Real32 + 0.5f);
}

inline uint32_t
RoundReal32ToUInt32(real32 Real32)
{
	return (uint32_t)(Real32 + 0.5f);
}

inline uint32_t
TruncateReal32ToUInt32(real32 Real32)
{
	return (uint32_t)(Real32);
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

inline tile_map *
GetTileMap(world *World, int32_t TileMapX, int32_t TileMapY)
{
	tile_map *TileMap = 0;

	if((TileMapX >= 0) && (TileMapX < World->TileMapCountX) &&
	   (TileMapY >= 0) && (TileMapY < World->TileMapCountY))
	{
		TileMap = &World->TileMaps[TileMapY * World->TileMapCountX + TileMapX];
	}
	return TileMap;
}

inline uint32_t
GetTileValueUnchecked(tile_map *TileMap, int32_t TileX, int32_t TileY)
{
	return TileMap->Tiles[TileY*TileMap->CountX + TileX];
}

internal bool32
IsTileMapPointEmpty(tile_map *TileMap, real32 TestX, real32 TestY)
{
	bool32 Empty = false;

	int32_t PlayerTileX = TruncateReal32ToUInt32((TestX - TileMap->UpperLeftX) / TileMap->TileWidth);
	int32_t PlayerTileY = TruncateReal32ToUInt32((TestY - TileMap->UpperLeftY) / TileMap->TileHeight);

	if ((PlayerTileX >= 0) && (PlayerTileX <= TileMap->CountX) &&
		(PlayerTileY >= 0) && (PlayerTileY <= TileMap->CountY))
	{
		uint32_t TileMapValue = GetTileValueUnchecked(TileMap, PlayerTileX, PlayerTileY);
		Empty = (TileMapValue == 0);
	}
	return Empty;
}

internal bool32
IsWorldPointEmpty(world *World, int32_t TileMapX, int32_t TileMapY, real32 TestX, real32 TestY)
{
	bool32 Empty = false;

	tile_map *TileMap = GetTileMap(World, TileMapX, TileMapY);
	if(TileMap)
	{
		int32_t PlayerTileX = TruncateReal32ToUInt32((TestX - TileMap->UpperLeftX) / TileMap->TileWidth);
		int32_t PlayerTileY = TruncateReal32ToUInt32((TestY - TileMap->UpperLeftY) / TileMap->TileHeight);

		if ((PlayerTileX >= 0) && (PlayerTileX <= TileMap->CountX) &&
			(PlayerTileY >= 0) && (PlayerTileY <= TileMap->CountY))
		{
			uint32_t TileMapValue = GetTileValueUnchecked(TileMap, PlayerTileX, PlayerTileY);
			Empty = (TileMapValue == 0);
		}
	}
	return Empty;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	Assert((&Input->Controllers[0].Back - &Input->Controllers[0].Buttons[0]) ==
			(ArrayCount(Input->Controllers[0].Buttons) - 1));
	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

#define TILE_MAP_COUNT_X 17
#define TILE_MAP_COUNT_Y 9

	uint32_t Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0},
		{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
		{1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1}
	};

	uint32_t Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1}
	};

	uint32_t Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
		{1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1}
	};

	uint32_t Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
		{1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1}
	};

	tile_map TileMaps[2][2];
	TileMaps[0][0].CountX = TILE_MAP_COUNT_X;
	TileMaps[0][0].CountY = TILE_MAP_COUNT_Y;
	TileMaps[0][0].UpperLeftX = -30;
	TileMaps[0][0].UpperLeftY = 0;
	TileMaps[0][0].TileHeight = 60;
	TileMaps[0][0].TileWidth =  60;
	TileMaps[0][0].Tiles = (uint32_t *)Tiles00;

	TileMaps[0][1] = TileMaps[0][0];
	TileMaps[0][1].Tiles = (uint32_t *)Tiles01;

	TileMaps[1][0] = TileMaps[0][0];
	TileMaps[1][0].Tiles = (uint32_t *)Tiles10;

	TileMaps[1][1] = TileMaps[0][0];
	TileMaps[1][1].Tiles = (uint32_t *)Tiles11;

	tile_map *TileMap = &TileMaps[0][0];

	world World;
	World.TileMaps = (tile_map *)TileMaps;
	World.TileMapCountX = 2;
	World.TileMapCountY = 2;

	real32 PlayerWidth = 0.75f*TileMap->TileWidth;
	real32 PlayerHeight = TileMap->TileHeight;

	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		Memory->IsInitialized = true;
		GameState->PlayerX = 150;
		GameState->PlayerY = 150;
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
				dPlayerY = -1.0f;
			}
			if(Controller->MoveDown.EndedDown)
			{
				dPlayerY = 1.0f;
			}
			if(Controller->MoveLeft.EndedDown)
			{
				dPlayerX = -1.0f;
			}
			if(Controller->MoveRight.EndedDown)
			{
				dPlayerX = 1.0f;
			}
			dPlayerX *= 128.0f;
			dPlayerY *= 128.0f;

			// TODO Diagonal will be faster! Fix once we have vectors
			real32 NewPlayerX = GameState->PlayerX + dPlayerX * Input->dtForFrame;
			real32 NewPlayerY = GameState->PlayerY + dPlayerY * Input->dtForFrame;

			if (IsTileMapPointEmpty(TileMap, NewPlayerX - 0.5f*PlayerWidth, NewPlayerY) &&
				IsTileMapPointEmpty(TileMap, NewPlayerX + 0.5f*PlayerWidth, NewPlayerY) &&
				IsTileMapPointEmpty(TileMap, NewPlayerX , NewPlayerY))
			{
				GameState->PlayerX = NewPlayerX;
				GameState->PlayerY = NewPlayerY;
			}
		}
	}

	DrawRect(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height, 1.0f, 0.5f, 0.0f);
	
	for(int Row = 0;
		Row < 9;
		++Row)
	{
		for(int Column = 0;
			Column < 17;
			++Column)
		{
			uint32_t TileID = GetTileValueUnchecked(TileMap, Column, Row);
			real32 Gray = 0.5f;
			if(TileID == 1)
			{
				Gray = 1.0f;
			}
			real32 MinX = TileMap->UpperLeftX + ((real32)Column) * TileMap->TileWidth;
			real32 MinY = TileMap->UpperLeftY + ((real32)Row) * TileMap->TileHeight;
			real32 MaxX = MinX + TileMap->TileWidth;
			real32 MaxY = MinY + TileMap->TileHeight;
			DrawRect(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
			//++UpperLeftX;
		}
		//++UpperLeftY;
		//UpperLeftX = -30;
	}

	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;
	real32 PlayerLeft = GameState->PlayerX - 0.5f*PlayerWidth;
	real32 PlayerTop = GameState->PlayerY - PlayerHeight;
	DrawRect(Buffer, 
			 PlayerLeft, PlayerTop,
			 PlayerLeft + PlayerWidth, PlayerTop + PlayerHeight,
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
