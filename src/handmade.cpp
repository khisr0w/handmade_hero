#include "handmade.h"
#include "handmade_tile.cpp"
#include "handmade_random.h"

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

internal void
DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap, real32 RealX, real32 RealY)
{
	int32_t MinX = RoundReal32ToInt32(RealX);
	int32_t MinY = RoundReal32ToInt32(RealY);
	int32_t MaxX = RoundReal32ToInt32(RealX + Bitmap->Width);
	int32_t MaxY = RoundReal32ToInt32(RealY + Bitmap->Height);

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

	// TODO SourceRow needs to be changed for clipping
	uint32_t *SourceRow = Bitmap->Pixels + Bitmap->Width*(Bitmap->Height - 1);
	uint8_t *DestRow = ((uint8_t *)Buffer->Memory +
								   MinX*Buffer->BytesPerPixel +
								   MinY*Buffer->Pitch);

	for(int Y = MinY;
		 Y < MaxY;
		 ++Y)
	{
		uint32_t *Dest = (uint32_t *)DestRow;
		uint32_t *Source = SourceRow;

		for(int X = MinX;
			X < MaxX;
			++X)
		{
			*Dest++ = *Source++;
		}

		DestRow += Buffer->Pitch;
		SourceRow -= Bitmap->Width;
	}
}

#pragma pack(push, 1)
struct bitmap_header
{
	uint16_t FileType; 
	uint32_t FileSize;
	uint16_t Reserved1;
	uint16_t Reserved2;
	uint32_t BitmapOffset;
	uint32_t Size; 
	int32_t Width;
	int32_t Height;
	uint16_t Planes;
	uint16_t BitsPerPixel;
	uint32_t Compression;
	uint32_t SizeOfBitmap;
	int32_t HorzResolution;
	int32_t VertResolution;
	uint32_t ColorsUsed;
	uint32_t ColorsImportant;

	uint32_t RedMask;
	uint32_t GreenMask;
	uint32_t BlueMask;
};
#pragma pack(pop)

internal loaded_bitmap
DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *Filename)
{
	loaded_bitmap Result = {};

	// NOTE Byte Order in memory is BB GG RR AA, bottom up
	// NOTE In casey's assets the BMP memory order is AA BB GG RR
	// in little endian -> 0xRRGGBBAA

	debug_read_file_result ReadResult = ReadEntireFile(Thread, Filename);
	if(ReadResult.ContentsSize != 0)
	{
		bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
		uint32_t *Pixels = (uint32_t *)((uint8_t *)ReadResult.Contents + Header->BitmapOffset);
		Result.Pixels = Pixels;
		Result.Width = Header->Width;
		Result.Height = Header->Height;

		// NOTE BMP files van go both ways in terms of signage of the height;
		// height will be negative for top-down
		// Also there can be compression as well, so don't think this actual BMP loader.

		uint32_t *SourceDest = Pixels;
		for(int32_t Y = 0;
			Y < Header->Height;
			++Y)
		{
			for(int32_t X = 0;
				X < Header->Width;
				++X)
			{
				*SourceDest = (*SourceDest >> 8) | (*SourceDest << 24);
				++SourceDest;
			}
		}
	}

	return Result;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	Assert((&Input->Controllers[0].Back - &Input->Controllers[0].Buttons[0]) ==
			(ArrayCount(Input->Controllers[0].Buttons) - 1));
	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	real32 PlayerHeight = 1.4f;
	real32 PlayerWidth = 0.75f * PlayerHeight;
	
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		GameState->BackDrop = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile,
				"handmade_hero_legacy_art/early_data/test/test_background.bmp" /* "structured_art.bmp"*/);
		GameState->HeroHead = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile,
				"handmade_hero_legacy_art/early_data/test/test_hero_front_head.bmp");
		GameState->HeroCape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile,
				"handmade_hero_legacy_art/early_data/test/test_hero_front_cape.bmp");
		GameState->HeroTorso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile,
				"handmade_hero_legacy_art/early_data/test/test_hero_front_torso.bmp");

		GameState->PlayerP.AbsTileX = 6;
		GameState->PlayerP.AbsTileY = 6;
		GameState->PlayerP.AbsTileZ = 0;
		GameState->PlayerP.OffsetX = 0.5f;
		GameState->PlayerP.OffsetY = 0.5f;
		InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
						(uint8_t *)Memory->PermanentStorage + sizeof(game_state));

		GameState->World = PushStruct(&GameState->WorldArena, world);
		world *World = GameState->World;
		World->TileMap = PushStruct(&GameState->WorldArena, tile_map);

		tile_map *TileMap = World->TileMap;

		TileMap->ChunkShift = 4;
		TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;
		TileMap->ChunkDim = (1 << TileMap->ChunkShift);
		TileMap->TileSideInMeters = 1.4f;

		TileMap->TileChunkCountX = 128;
		TileMap->TileChunkCountY = 128;
		TileMap->TileChunkCountZ = 2;

		TileMap->TileChunks = PushArray(&GameState->WorldArena,
										TileMap->TileChunkCountX*
										TileMap->TileChunkCountX*
										TileMap->TileChunkCountZ,
										tile_chunk);

		uint32_t RandomNumberIndex = 0;
		uint32_t TilesPerWidth = 17;
		uint32_t TilesPerHeight = 9;
		uint32_t ScreenX = 0;
		uint32_t ScreenY = 0;
		uint32_t AbsTileZ = 0;

		bool32 DoorLeft = false;
		bool32 DoorRight = false;
		bool32 DoorTop = false;
		bool32 DoorBottom = false;
		bool32 DoorUp = false;
		bool32 DoorDown = false;
		for(uint32_t ScreenIndex = 0;
			ScreenIndex < 100;
			++ScreenIndex)
		{
			Assert(RandomNumberIndex < ArrayCount(RandomNumberTable));
			uint32_t RandomChoice;
			if (DoorUp || DoorDown)
			{
				RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;
			}
			else
			{
				RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
			}

			bool32 CreatedZDoor = false;
			if(RandomChoice == 2)
			{
				CreatedZDoor = true;
				if(AbsTileZ == 0)
				{
					DoorUp = true;
				}
				else
				{
					DoorDown = true;
				}
			}
			else if(RandomChoice == 1)
			{
				DoorRight = true;
			}
			else
			{
				DoorTop = true;
			}

			for(uint32_t TileY = 0;
				TileY < TilesPerHeight;
				++TileY)
			{
				for(uint32_t TileX = 0;
					TileX < TilesPerWidth;
					++TileX)
				{
					uint32_t AbsTileX = ScreenX*TilesPerWidth + TileX;
					uint32_t AbsTileY = ScreenY*TilesPerHeight + TileY;

					uint32_t TileValue = 1;
					if((TileX == 0) && (!DoorLeft || (TileY != (TilesPerHeight/2))))
					{
						TileValue = 2;
					}
					if((TileX == (TilesPerWidth - 1)) && (!DoorRight || (TileY != (TilesPerHeight/2))))
					{
						TileValue = 2;
					}

					if((TileY == 0) && (!DoorBottom || (TileX != (TilesPerWidth/2))))
					{
						TileValue = 2;
					}
					if((TileY == TilesPerHeight - 1) && (!DoorTop || (TileX != (TilesPerWidth/2))))
					{
						TileValue = 2;
					}
					if((TileX == 10) && (TileY == 6))
					{
						if(DoorUp)
						{
							TileValue = 3;
						}
						else if(DoorDown)
						{
							TileValue = 4;
						}
					}

					SetTileValue(&GameState->WorldArena, World->TileMap,
							AbsTileX, AbsTileY, AbsTileZ, TileValue);
				}
			}

			DoorLeft = DoorRight;
			DoorBottom = DoorTop;

			if(CreatedZDoor)
			{
				DoorDown = !DoorDown;
				DoorUp = !DoorUp;
			}
			else
			{
				DoorUp = false;
				DoorDown = false;
			}

			DoorRight = false;
			DoorTop = false;

			if(RandomChoice == 2)
			{
				if(AbsTileZ == 0)
				{
					AbsTileZ = 1;
				}
				else
				{
					AbsTileZ = 0;
				}
			}
			else if(RandomChoice == 1)
			{
				ScreenX += 1;
			}
			else
			{
				ScreenY += 1;
			}
		}

		Memory->IsInitialized = true;
	}

	world *World = GameState->World;
	tile_map *TileMap = World->TileMap;

	int32_t TileSideInPixels = 60;
	real32 MetersToPixels = (real32)TileSideInPixels / (real32)TileMap->TileSideInMeters;

	real32 LowerLeftX = -(real32)TileSideInPixels / 2;
	real32 LowerLeftY = (real32)Buffer->Height;

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
			real32 PlayerSpeed = 2.0f;
			if(Controller->ActionUp.EndedDown)
			{
				PlayerSpeed = 10.0f;
			}

			dPlayerX *= PlayerSpeed;
			dPlayerY *= PlayerSpeed;

			// TODO Diagonal will be faster! Fix once we have vectors
			tile_map_position NewPlayerP = GameState->PlayerP;
			NewPlayerP.OffsetX += dPlayerX * Input->dtForFrame;
			NewPlayerP.OffsetY += dPlayerY * Input->dtForFrame;
			NewPlayerP = RecanonicalizePosition(TileMap, NewPlayerP);

			tile_map_position PlayerLeft = NewPlayerP;
			PlayerLeft.OffsetX -= 0.5f*PlayerWidth;
			PlayerLeft = RecanonicalizePosition(TileMap, PlayerLeft);

			tile_map_position PlayerRight = NewPlayerP;
			PlayerRight.OffsetX += 0.5f*PlayerWidth;
			PlayerRight = RecanonicalizePosition(TileMap, PlayerRight);

			if (IsTileMapPointEmpty(TileMap, NewPlayerP) &&
				IsTileMapPointEmpty(TileMap, PlayerLeft) &&
				IsTileMapPointEmpty(TileMap, PlayerRight))
			{
				if(!AreOnSameTile(&GameState->PlayerP, &NewPlayerP))
				{
					uint32_t NewTileValue = GetTileValue(TileMap, NewPlayerP);
					if(NewTileValue == 3)
					{
						++NewPlayerP.AbsTileZ;
					}
					else if(NewTileValue == 4)
					{
						--NewPlayerP.AbsTileZ;
					}
				}
				GameState->PlayerP = NewPlayerP;
			}
		}
	}

	DrawBitmap(Buffer, &GameState->BackDrop, 0, 0);

	real32 ScreenCenterX = 0.5f * (real32)Buffer->Width;
	real32 ScreenCenterY = 0.5f * (real32)Buffer->Height;

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
			uint32_t TileID = GetTileValue(TileMap, Column, Row, GameState->PlayerP.AbsTileZ);
			if(TileID > 1)
			{
				real32 Gray = 0.5f;
				if(TileID == 2)
				{
					Gray = 1.0f;
				}

				if(TileID > 2)
				{
					Gray = 0.25f;
				}

				if((Column == GameState->PlayerP.AbsTileX) &&
						(Row == GameState->PlayerP.AbsTileY))
				{
					Gray = 0.f;
				}

				real32 CenX = ScreenCenterX + ((real32)RelColumn)*TileSideInPixels - GameState->PlayerP.OffsetX*MetersToPixels;
				real32 CenY = ScreenCenterY - ((real32)RelRow)*TileSideInPixels + GameState->PlayerP.OffsetY*MetersToPixels;
				real32 MinX = CenX - 0.5f*TileSideInPixels;
				real32 MinY = CenY - 0.5f*TileSideInPixels;
				real32 MaxX = CenX + 0.5f*TileSideInPixels;
				real32 MaxY = CenY + 0.5f*TileSideInPixels;
				DrawRect(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
			}
		}
	}

	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;
	real32 PlayerLeft = ScreenCenterX
						// + MetersToPixels*GameState->PlayerP.OffsetX
						- 0.5f*MetersToPixels*PlayerWidth;
	real32 PlayerTop = ScreenCenterY
					   // - MetersToPixels*GameState->PlayerP.OffsetY
					   - MetersToPixels*PlayerHeight;
	DrawRect(Buffer, 
			 PlayerLeft, PlayerTop,
			 PlayerLeft + PlayerWidth * MetersToPixels,
			 PlayerTop + PlayerHeight*MetersToPixels,
			 PlayerR, PlayerG, PlayerB);
	// DrawBitmap(Buffer, &GameState->HeroHead, PlayerLeft, PlayerTop);
	DrawBitmap(Buffer, &GameState->HeroHead, 0, 0);
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
