// This is the header file for the platform independent layer
#if !defined(HANDMADE_H)

/*
	HANDMADE_INTERNAL:
	0 - Build for the public release
	1 - Build for developer only

	HANDMADE_SLOW:
	0 - No Slow Code allowed
	1 - Slow code allowed
*/

#if HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int*)0 = 0;}
#else
#define Assert(Expression)
#endif

inline uint32_t SafeTruncateUInt64 (uint64_t Value) {

	Assert(Value <= 0xFFFFFFFF);
	return (uint32_t)Value;
}
// TODO should this always be 64-bit?
#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

/* 
   TODO Services that the platform layer provides to the game
*/

#if HANDMADE_INTERNAL
/*
   NOTE These are not for doing anything in the shipping game - they are blocking and the write doesn't
   protect against lost data!
*/

struct debug_read_file_result {
	uint32_t ContentsSize;
	void* Contents;
};

internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal void DEBUGPlatformFreeFileMemory(void *Memory);

internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32_t MemorySize, void *Memory);
#endif

/* 
   Services that the game provide to the platform layer
 */

// FOUR THINGS - Timing, Controller/Keyboard, Bitmap buffer to use, Sound Buffer to use

struct game_sound_output_buffer {

	int SampleCountToOutput;
	int16_t *Samples;
	int SamplesPerSecond; 
};

struct game_offscreen_buffer {

	void *Memory;
	int Width; 
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct game_button_state {

	int HalfTransitionCount;
	bool32 EndedDown;
};

struct game_controller_input {

	bool32 IsConnected;
	bool32 IsAnalog;
	real32 StickAverageX;
	real32 StickAverageY;

	union {
		game_button_state Buttons[12];
		struct {
			game_button_state MoveUp;
			game_button_state MoveDown;
			game_button_state MoveLeft;
			game_button_state MoveRight;

			game_button_state ActionUp;
			game_button_state ActionDown;
			game_button_state ActionLeft;
			game_button_state ActionRight;

			game_button_state LeftShoulder;
			game_button_state RightShoulder;

			game_button_state Start;
			game_button_state Back;

			// WARNING All buttons must be added above this line, the Terminator button must not
			// be used except for assertions
			game_button_state Terminator;
		};
	};
};

struct game_input {

	// TODO Insert game clock values in here
	game_controller_input Controllers[5];
};

inline game_controller_input *GetController (game_input *Input, int unsigned ControllerIndex)
{
	Assert(ControllerIndex < ArrayCount(Input->Controllers));

	game_controller_input *Result = &Input->Controllers[ControllerIndex];
	return Result;
}

struct game_memory {
	bool32 IsInitialized;

	uint64_t PermanentStorageSize;
	void *PermanentStorage; // This memory is required to be zero at startup

	uint64_t TransientStorageSize;
	void *TransientStorage; // This memory is required to be zero at startup
};

internal void 
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer, 
		game_sound_output_buffer *SoundBuffer);

struct game_state {
	int ToneHz;
	int GreenOffset;
	int BlueOffset;
};

#define HANDMADE_H
#endif
