#include "handmade.h"

internal void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz) {

	local_persist real32 tSine;
	int16_t ToneVolume = 3000;
	int WavePeriod =SoundBuffer->SamplesPerSecond/ToneHz;

	int16_t *SampleOut = SoundBuffer->Samples;

	for (int SampleIndex = 0; 
			SampleIndex < SoundBuffer->SampleCountToOutput; 
			++SampleIndex) {

		real32 SineValue = sinf(tSine);
		int16_t SampleValue = (int16_t)(SineValue * ToneVolume); 
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;

		tSine += 2.0f*PI32 * 1.0f / (real32)WavePeriod;
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
			// Coloring scheme is BRG because fuck windows
			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer->Pitch;
	}
}

internal void 
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer, 
		game_sound_output_buffer *SoundBuffer) {

	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if(!Memory->IsInitialized) {

		char *Filename = __FILE__;

		debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
		if (File.Contents) {

			DEBUGPlatformWriteEntireFile("Handmade_COPY.cpp", File.ContentsSize, File.Contents);
			DEBUGPlatformFreeFileMemory(File.Contents);
		}

		GameState->ToneHz = 256;

		// TODO This may be appropriate for the platform layer
		Memory->IsInitialized = true;
	}

	game_controller_input *Input0 = &Input->Controllers[0];
	if(Input0->IsAnalog) {

		// Do the analogue tunning
		GameState->ToneHz = 256 + (int)(128.0f * (Input0->EndY));
		GameState->BlueOffset += (int)(4.0f*Input0->EndX);

	} else {

		// Digital input tunning
	}

	// Input.AButtonEndedDown;
	// Input.AButtonHalfTransitionCount;
	if (Input0->Down.EndedDown) {

		GameState->GreenOffset += 1;
	}


	// TODO Allow sample offsets here for more robust platform options
	GameOutputSound(SoundBuffer, GameState->ToneHz);
	RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
}
