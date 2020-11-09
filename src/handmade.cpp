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
DrawRectangle(game_offscreen_buffer *Buffer, v2 vMin, v2 vMax, real32 R, real32 G, real32 B)
{
	int32_t MinX = RoundReal32ToInt32(vMin.X);
	int32_t MinY = RoundReal32ToInt32(vMin.Y);
	int32_t MaxX = RoundReal32ToInt32(vMax.X);
	int32_t MaxY = RoundReal32ToInt32(vMax.Y);

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
DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap,
		   real32 RealX, real32 RealY,
		   int32_t AlignX = 0, int32_t AlignY = 0)
{
	RealX -= (real32)AlignX;
	RealY -= (real32)AlignY;

	int32_t MinX = RoundReal32ToInt32(RealX);
	int32_t MinY = RoundReal32ToInt32(RealY);
	int32_t MaxX = RoundReal32ToInt32(RealX + Bitmap->Width);
	int32_t MaxY = RoundReal32ToInt32(RealY + Bitmap->Height);

	int32_t SourceOffsetX = 0;
	if(MinX < 0)
	{
		SourceOffsetX = -MinX;
		MinX = 0;
	}

	int32_t SourceOffsetY = 0;
	if(MinY < 0)
	{
		SourceOffsetY = -MinY;
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
	SourceRow += -SourceOffsetY*Bitmap->Width + SourceOffsetX;

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
			real32 A = (real32)((*Source >> 24) & 0xFF) / 255.0f;
			real32 SR = (real32)((*Source >> 16) & 0xFF);
			real32 SG = (real32)((*Source >> 8) & 0xFF);
			real32 SB = (real32)((*Source >> 0) & 0xFF);

			real32 DR = (real32)((*Dest >> 16) & 0xFF);
			real32 DG = (real32)((*Dest >> 8) & 0xFF);
			real32 DB = (real32)((*Dest >> 0) & 0xFF);

			// TODO Premultiplied alpha
			real32 R = (1.0f - A)*DR + A*SR;
			real32 G = (1.0f - A)*DG + A*SG;
			real32 B = (1.0f - A)*DB + A*SB;

			*Dest = (((uint32_t)(R + 0.5f) << 16) |
					   ((uint32_t)(G + 0.5f) << 8) |
					   ((uint32_t)(B + 0.5f) << 0));
			++Dest;
			++Source;
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

	// NOTE Byte Order in memory is determined by the header itself, so we have to
	// read out the masks and convert the pixels.

	debug_read_file_result ReadResult = ReadEntireFile(Thread, Filename);
	if(ReadResult.ContentsSize != 0)
	{
		bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
		uint32_t *Pixels = (uint32_t *)((uint8_t *)ReadResult.Contents + Header->BitmapOffset);
		Result.Pixels = Pixels;
		Result.Width = Header->Width;
		Result.Height = Header->Height;

		Assert(Header->Compression == 3);

		// NOTE BMP files van go both ways in terms of signage of the height;
		// height will be negative for top-down
		// Also there can be compression as well, so don't think this actual BMP loader.
		uint32_t RedMask = Header->RedMask;
		uint32_t GreenMask = Header->GreenMask;
		uint32_t BlueMask = Header->BlueMask;
		uint32_t AlphaMask = ~(RedMask | GreenMask | BlueMask);

		bit_scan_result RedScan = FindLeastSignificantSetBit(RedMask);
		bit_scan_result GreenScan = FindLeastSignificantSetBit(GreenMask);
		bit_scan_result BlueScan = FindLeastSignificantSetBit(BlueMask);
		bit_scan_result AlphaScan = FindLeastSignificantSetBit(AlphaMask);

		Assert(RedScan.Found);
		Assert(GreenScan.Found);
		Assert(BlueScan.Found);
		Assert(AlphaScan.Found);

		int32_t RedShift = 16 - (int32_t)RedScan.Index;
		int32_t GreenShift = 8 - (int32_t)GreenScan.Index;
		int32_t BlueShift = 0 - (int32_t)BlueScan.Index;
		int32_t AlphaShift = 24 - (int32_t)AlphaScan.Index;

		uint32_t *SourceDest = Pixels;
		for(int32_t Y = 0;
			Y < Header->Height;
			++Y)
		{
			for(int32_t X = 0;
				X < Header->Width;
				++X)
			{
				uint32_t C = *SourceDest;
#if 0
				*SourceDest++ = ((((C >> AlphaShift.Index) & 0xFF) << 24) |
								 (((C >> RedShift.Index) & 0xFF) << 16) |
								 (((C >> GreenShift.Index) & 0xFF) << 8) |
								 (((C >> BlueShift.Index) & 0xFF) << 0));
#else
				*SourceDest++ = (RotateLeft(C & RedMask, RedShift) |
								 RotateLeft(C & GreenMask, GreenShift) |
								 RotateLeft(C & BlueMask, BlueShift) |
								 RotateLeft(C & AlphaMask, AlphaShift));
#endif
			}
		}
	}

	return Result;
}

inline entity
GetEntity(game_state *GameState, entity_residence Residence, uint32_t Index)
{
	entity Entity {};

	if((Index > 0) && (Index < GameState->EntityCount))
	{
		Entity.Residence = Residence;
		Entity.Dormant = &GameState->DormantEntities[Index];
		Entity.Low = &GameState->LowEntities[Index];
		Entity.High	 = &GameState->HighEntities[Index];
	}

	return Entity;
}

internal void
ChangeEntityResidence(game_state *GameState, entity *Entity, entity_residence Residence)
{
}

internal uint32_t
AddEntity(game_state *GameState)
{
	uint32_t EntityIndex = GameState->EntityCount++;

	Assert(GameState->EntityCount < ArrayCount(GameState->DormantEntities));
	Assert(GameState->EntityCount < ArrayCount(GameState->LowEntities));
	Assert(GameState->EntityCount < ArrayCount(GameState->HighEntities));

	GameState->EntityResidence[EntityIndex] = EntityResidence_Dormant;
	GameState->DormantEntities[EntityIndex] = {};
	GameState->LowEntities[EntityIndex] = {};
	GameState->HighEntities[EntityIndex] = {};

	return EntityIndex;
}

internal void
InitializePlayer(game_state *GameState, uint32_t EntityIndex)
{
	entity Entity = GetEntity(GameState, EntityResidence_Dormant, EntityIndex);

	Entity.Dormant->P.AbsTileX = 6;
	Entity.Dormant->P.AbsTileY = 6;
	Entity.Dormant->P.AbsTileZ = 0;
	Entity.Dormant->P.Offset_.X = 0;
	Entity.Dormant->P.Offset_.Y = 0;
	Entity.Dormant->Height = 0.5f;
	Entity.Dormant->Width = 1.0f;

	ChangeEntityResidence(GameState, &Entity, EntityResidence_High);

	if(GetEntity(GameState, EntityResidence_Dormant, GameState->CameraFollowingEntityIndex).Residence ==
	   EntityResidence_Nonexistent)
	{
		GameState->CameraFollowingEntityIndex = EntityIndex;
	}
}

internal bool32
TestWall(real32 WallX, real32 RelX, real32 RelY, real32 PlayerDeltaX, real32 PlayerDeltaY,
		 real32 *tMin, real32 MinY, real32 MaxY)
{
	bool32 Hit = false;
	real32 tEpsilon = 0.0001f;
	if(PlayerDeltaX != 0.0f)
	{
		real32 tResult = (WallX - RelX) / PlayerDeltaX;
		real32 Y = RelY + tResult*PlayerDeltaY;
		if((tResult >= 0.0f) && (*tMin > tResult))
		{
			if((Y >= MinY ) && (Y <= MaxY))
			{
				*tMin = Maximum(0.0f, tResult - tEpsilon);
				Hit = true;
			}
		}
	}

	return Hit;
}

internal void
MovePlayer(game_state *GameState, entity Entity, real32 dt, v2 ddP)
{
	tile_map *TileMap = GameState->World->TileMap;

	real32 ddPLengthSq = LengthSq(ddP);
	if(ddPLengthSq > 1.0f)
	{
		ddP *= (1.0f / SquareRoot(ddPLengthSq));
	}

	real32 PlayerSpeed = 50.0f; // m/s^2
	ddP *= PlayerSpeed;

	// TODO ODE here!
	ddP += -8.0f * Entity.High->dP;

	v2 OldPlayerP = Entity.High->P;
	v2 PlayerDelta = (0.5f*ddP*Square(dt) + Entity.High->dP*dt);
	Entity.High->dP = ddP*dt + Entity.High->dP;
	v2 NewPlayerP = OldPlayerP + PlayerDelta;

#if 0

	uint32_t MinTileX = Minimum(OldPlayerP.AbsTileX, NewPlayerP.AbsTileX);
	uint32_t MinTileY = Minimum(OldPlayerP.AbsTileY, NewPlayerP.AbsTileY);
	uint32_t MaxTileX = Maximum(OldPlayerP.AbsTileX, NewPlayerP.AbsTileX);
	uint32_t MaxTileY = Maximum(OldPlayerP.AbsTileY, NewPlayerP.AbsTileY);

	uint32_t EntityTileWidth = CeilReal32ToInt32(Entity.High->Width / TileMap->TileSideInMeters);
	uint32_t EntityTileHeight = CeilReal32ToInt32(Entity.High->Height / TileMap->TileSideInMeters);

	MinTileX -= EntityTileWidth;
	MinTileY -= EntityTileHeight;
	MaxTileX += EntityTileWidth;
	MaxTileY += EntityTileHeight;

	uint32_t AbsTileZ = Entity.High->P.AbsTileZ;

	real32 tRemaining = 1.0f;
	for(uint32_t Iteration = 0;
		(Iteration) < 4 && (tRemaining > 0.0f);
		++Iteration)
	{
		real32 tMin = 1.0f;
		v2 WallNormal = {};

		Assert((MaxTileX - MinTileX) < 32);
		Assert((MaxTileY - MinTileY) < 32);

		for(uint32_t AbsTileY = MinTileY;
			AbsTileY <= MaxTileY;
			++AbsTileY)
		{
			for(uint32_t AbsTileX = MinTileX;
				AbsTileX <= MaxTileX;
				++AbsTileX)
			{
				tile_map_position TestTileP = CenteredTilePoint(AbsTileX, AbsTileY, AbsTileZ);
				uint32_t TileValue = GetTileValue(TileMap, TestTileP);
				if(!IsTileValueEmpty(TileValue))
				{
					real32 DiameterW = TileMap->TileSideInMeters + Entity.High->Width;
					real32 DiameterH = TileMap->TileSideInMeters + Entity.High->Height;
					v2 MinCorner = -0.5f*v2{DiameterW, DiameterH};
					v2 MaxCorner = 0.5f*v2{DiameterW, DiameterH};

					tile_map_difference RelOldPlayerP = Subtract(TileMap, &Entity.High->P, &TestTileP);
					v2 Rel = RelOldPlayerP.dXY;

					// NOTE: when will the player hit the wall =>  t = (wx - p0x) / dx
					// NOTE: Test all four walls and take the minimum t of it.
					if(TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin,
								MinCorner.Y, MaxCorner.Y))
					{
						WallNormal = v2{-1, 0};
					}
					if(TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin,
								MinCorner.Y, MaxCorner.Y))
					{
						WallNormal = v2{1, 0};
					}
					if(TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin,
								MinCorner.X, MaxCorner.X))
					{
						WallNormal = v2{0, -1};
					}
					if(TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin,
								MinCorner.X, MaxCorner.X))
					{
						WallNormal = v2{0, 1};
					}
				}
			}
		}

		Entity.High->P = Offset(TileMap, Entity.High->P, tMin*PlayerDelta);
		Entity.High->dP = Entity.High->dP - 1*Inner(Entity.High->dP, WallNormal)*WallNormal;
		PlayerDelta = PlayerDelta - 1*Inner(PlayerDelta, WallNormal)*WallNormal; 
		tRemaining -= tRemaining*tMin;
	}

	//
	// NOTE Update z tile based on player movement
	//
	if(!AreOnSameTile(&OldPlayerP, &Entity.High->P))
	{
		uint32_t NewTileValue = GetTileValue(TileMap, Entity.High->P);
		if(NewTileValue == 3)
		{
			++Entity.High->P.AbsTileZ;
		}
		else if(NewTileValue == 4)
		{
			--Entity.High->P.AbsTileZ;
		}
	}

	if((Entity.High->dP.X == 0.0f ) > (Entity.High->dP.Y == 0.0f ))
	{
		// NOTE Should not be handled
	}

	else if(AbsoluteValue(Entity.High->dP.X) > AbsoluteValue(Entity.High->dP.Y))
	{
		if(Entity.High->dP.X > 0)
		{
			Entity.High->FacingDirection = 0;
		}
		else
		{
			Entity.High->FacingDirection = 2;
		}
	}
	else if(AbsoluteValue(Entity.High->dP.X) < AbsoluteValue(Entity.High->dP.Y))
	{
		if(Entity.High->dP.Y > 0)
		{
			Entity.High->FacingDirection = 1;
		}
		else
		{
			Entity.High->FacingDirection = 3;
		}
	}
#endif
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	Assert((&Input->Controllers[0].Back - &Input->Controllers[0].Buttons[0]) ==
			(ArrayCount(Input->Controllers[0].Buttons) - 1));
	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		// NOTE Reserve the entity slot 0 for the NULL entity
		AddEntity(GameState);

		GameState->BackDrop = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile,
				"handmade_hero_legacy_art/early_data/test/test_background.bmp" /* "structured_art.bmp"*/);

		hero_bitmaps *Bitmap;

		Bitmap = GameState->HeroBitmaps;
		Bitmap-> Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_right_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_right_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_right_torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;

		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_back_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_back_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_back_torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;

		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_left_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_left_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_left_torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;

		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_front_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_front_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "handmade_hero_legacy_art/early_data/test/test_hero_front_torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;

		GameState->CameraP.AbsTileX = 17/2;
		GameState->CameraP.AbsTileY = 9/2;

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
#if 0
		// TODO waiting for full sparseness;
		uint32_t ScreenX = INT32_MAX / 2;
		uint32_t ScreenY = INT32_MAX / 2;
#else
		uint32_t ScreenX = 0;
		uint32_t ScreenY = 0;
#endif
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
		entity ControllingEntity = GetEntity(GameState,
											 EntityResidence_High,
											 GameState->PlayerIndexForController[ControllerIndex]);
		if(ControllingEntity.Residence != EntityResidence_Nonexistent)
		{
			v2 ddP = {};
			if(Controller->IsAnalog)
			{
				ddP = v2{Controller->StickAverageX, Controller->StickAverageY};
			} 
			else
			{
				// NOTE Digital input tunning
				if(Controller->MoveUp.EndedDown)
				{
					ddP.Y = 1.0f;
				}
				if(Controller->MoveDown.EndedDown)
				{
					ddP.Y = -1.0f;
				}
				if(Controller->MoveLeft.EndedDown)
				{
					ddP.X = -1.0f;
				}
				if(Controller->MoveRight.EndedDown)
				{
					ddP.X = 1.0f;
				}
			}

			MovePlayer(GameState, ControllingEntity, Input->dtForFrame, ddP);
		}
		else
		{
			if(Controller->Start.EndedDown)
			{
				uint32_t EntityIndex = AddEntity(GameState);
				InitializePlayer(GameState, EntityIndex);
				GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
			}
		}
	}

	entity CameraFollowingEntity = GetEntity(GameState, EntityResidence_High,
											 GameState->CameraFollowingEntityIndex);
	if(CameraFollowingEntity.Residence != EntityResidence_Nonexistent)
	{
		GameState->CameraP.AbsTileZ = CameraFollowingEntity.Dormant->P.AbsTileZ;
#if 0
		//
		// NOTE Update camera based on player movement
		//
		tile_map_difference Diff = Subtract(TileMap, &CameraFollowingEntity->P, &GameState->CameraP);
		if(Diff.dXY.X > (9.0f*TileMap->TileSideInMeters))
		{
			GameState->CameraP.AbsTileX += 17;
		}
		if(Diff.dXY.X < -(9.0f*TileMap->TileSideInMeters))
		{
			GameState->CameraP.AbsTileX -= 17;
		}
		if(Diff.dXY.Y > (5.0f*TileMap->TileSideInMeters))
		{
			GameState->CameraP.AbsTileY += 9;
		}
		if(Diff.dXY.Y < -(5.0f*TileMap->TileSideInMeters))
		{
			GameState->CameraP.AbsTileY -= 9;
		}
#endif
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
			uint32_t Column = GameState->CameraP.AbsTileX + RelColumn;
			uint32_t Row = GameState->CameraP.AbsTileY + RelRow;
			uint32_t TileID = GetTileValue(TileMap, Column, Row, GameState->CameraP.AbsTileZ);
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

				if((Column == GameState->CameraP.AbsTileX) &&
						(Row == GameState->CameraP.AbsTileY))
				{
					Gray = 0.f;
				}

				v2 TileSide = {0.5f*TileSideInPixels, 0.5f*TileSideInPixels};
				v2 Cen = {ScreenCenterX + ((real32)RelColumn)*TileSideInPixels - GameState->CameraP.Offset_.X*MetersToPixels,
						  ScreenCenterY - ((real32)RelRow)*TileSideInPixels + GameState->CameraP.Offset_.Y*MetersToPixels};
				v2 Min = Cen - TileSide;
				v2 Max = Cen + TileSide;
				DrawRectangle(Buffer, Min, Max, Gray, Gray, Gray);
			}
		}
	}

	for(uint32_t EntityIndex = 0;
		EntityIndex < GameState->EntityCount;
		++EntityIndex)
	{
		if(GameState->EntityResidence[EntityIndex] == EntityResidence_High)
		{
			high_entity *HighEntity = &GameState->HighEntities[EntityIndex];
			low_entity *LowEntity = &GameState->LowEntities[EntityIndex];
			dormant_entity *DormantEntity = &GameState->DormantEntities[EntityIndex];

			real32 PlayerR = 1.0f;
			real32 PlayerG = 1.0f;
			real32 PlayerB = 0.0f;
			real32 PlayerGroundPointX = ScreenCenterX + MetersToPixels*HighEntity->P.X; 
			real32 PlayerGroundPointY = ScreenCenterY - MetersToPixels*HighEntity->P.Y;
			v2 PlayerLeftTop = {PlayerGroundPointX - 0.5f*MetersToPixels*DormantEntity->Width,
				PlayerGroundPointY - 0.5f*MetersToPixels*DormantEntity->Height};
			v2 EntityWidthHeight = {DormantEntity->Width, DormantEntity->Height};
			DrawRectangle(Buffer, 
					PlayerLeftTop,
					PlayerLeftTop + MetersToPixels*EntityWidthHeight,
					PlayerR, PlayerG, PlayerB);

			hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[HighEntity->FacingDirection];
			DrawBitmap(Buffer, &HeroBitmaps->Torso, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX, HeroBitmaps->AlignY);
			DrawBitmap(Buffer, &HeroBitmaps->Cape, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX, HeroBitmaps->AlignY);
			DrawBitmap(Buffer, &HeroBitmaps->Head, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX, HeroBitmaps->AlignY);
		}
	}
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, 400);
}
