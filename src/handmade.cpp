#include "handmade.h"

internal void GameOutputSound(game_state * GameState, game_sound_output_buffer *SoundBuffer, int ToneHz) {

	int16_t ToneVolume = 3000;
	int WavePeriod =SoundBuffer->SamplesPerSecond/ToneHz;

	int16_t *SampleOut = SoundBuffer->Samples;

	for (int SampleIndex = 0; 
			SampleIndex < SoundBuffer->SampleCountToOutput; 
			++SampleIndex) {

#if 1
		real32 SineValue = sinf(GameState->tSine);
		int16_t SampleValue = (int16_t)(SineValue * ToneVolume); 
#else
		int16_t SampleValue = 0;
#endif
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;

		GameState->tSine += 2.0f*PI32 * 1.0f / (real32)WavePeriod;
		if(GameState->tSine > 2.0f*PI32)
		{
			GameState->tSine -= 2.0f*PI32;
		}
	}
}

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

internal void
RenderPlayer(game_offscreen_buffer *Buffer, int PlayerX, int PlayerY)
{
	uint8_t *EndofBuffer = (uint8_t *)Buffer->Memory + Buffer->BytesPerPixel * Buffer->Width * Buffer->Height;
	uint32_t Color = 0xFFFFFFFF;

	int Top = PlayerY;
	int Bottom = PlayerY+10;

	for (int X = PlayerX;
		 X < PlayerX+10;
		 ++X)
	{
		uint8_t *Pixel = ((uint8_t *)Buffer->Memory +
						  X*Buffer->BytesPerPixel +
						  Top*Buffer->Pitch);
		for (int Y = Top;
			 Y < Bottom;
			 ++Y)
		{
			if ((Pixel >= Buffer->Memory) &&
				(Pixel < EndofBuffer))
			{
				*(uint32_t *)Pixel = Color;
				Pixel += Buffer->Pitch;
			}
		}	
	}
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	Assert((&Input->Controllers[0].Back - &Input->Controllers[0].Buttons[0]) ==
			(ArrayCount(Input->Controllers[0].Buttons) - 1));
	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if(!Memory->IsInitialized) {

		char *Filename = __FILE__;

		debug_read_file_result File = Memory->DEBUGPlatformReadEntireFile(Thread, Filename);
		if (File.Contents) {

			Memory->DEBUGPlatformWriteEntireFile(Thread, "Handmade_COPY.cpp", File.ContentsSize, File.Contents);
			Memory->DEBUGPlatformFreeFileMemory(Thread, File.Contents);
		}

		GameState->ToneHz = 256;
		GameState->tSine = 0.0f;

		GameState->PlayerX = 100;
		GameState->PlayerY = 100;

		// TODO This may be appropriate for the platform layer
		Memory->IsInitialized = true;
	}

	for (int ControllerIndex = 0;
		 ControllerIndex < ArrayCount(Input->Controllers);
		 ++ControllerIndex)
	{
		game_controller_input *Controller = GetController(Input, ControllerIndex);
		if(Controller->IsAnalog) {

			// Do the analogue tunning
			GameState->ToneHz = 256 + (int)(128.0f * (Controller->StickAverageY));
			GameState->BlueOffset += (int)(4.0f*Controller->StickAverageX);

		} else {

			// Digital input tunning
			if (Controller->MoveLeft.EndedDown)
			{
				GameState->BlueOffset += 1;
			}
			if (Controller->MoveRight.EndedDown)
			{
				GameState->BlueOffset -= 1;
			}
		}

		// Input.AButtonEndedDown;
		// Input.AButtonHalfTransitionCount;
		
		GameState->PlayerX += (int)(4.0f*Controller->StickAverageX);
		GameState->PlayerY -= (int)(4.0f*Controller->StickAverageY /*+ 10.0f * sinf(GameState->tJump)*/);
		if (GameState->tJump > 0)
		{
			GameState->PlayerY += (int)(10.0f * sinf(0.5f * PI32 * GameState->tJump));
		}
		if (Controller->ActionDown.EndedDown) {

			GameState->tJump = 4.0;
		}
		GameState->tJump -= 0.033f;
	}
	RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
	RenderPlayer(Buffer, GameState->PlayerX, GameState->PlayerY);
	RenderPlayer(Buffer, Input->MouseX, Input->MouseY);

	for(int ButtonIndex = 0;
		ButtonIndex < ArrayCount(Input->MouseButtons);
		++ButtonIndex)
	{
		if(Input->MouseButtons[ButtonIndex].EndedDown)
		{
			RenderPlayer(Buffer, 10 + 20 * ButtonIndex, 10);
		}
	}
	
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, GameState->ToneHz);
}
