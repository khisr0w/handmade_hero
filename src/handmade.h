#if !defined(HANDMADE_H)
// This is the header file for the platform independent layer

/* 
   TODO Services that the platform layer provides to the game
*/


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
internal void 
GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset, int ToneHz);

#define HANDMADE_H
#endif
