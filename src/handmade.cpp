#include "handmade.h"

internal void RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset) {
    
    // TODO let's what the optimizer does

    uint8_t *Row = (uint8_t *)Buffer->Memory;
    for(int Y = 0; Y < Buffer->Height; ++Y) {

	uint32_t *Pixel = (uint32_t *)Row;
	for(int X = 0; X < Buffer->Width; ++X) {

	    uint8_t Blue = (X + XOffset);
	    uint8_t Green = (Y + YOffset);
	    // Coloring scheme is BRG because fuck windows
	    *Pixel++ = ((Green << 8) | Blue);
	}
	Row += Buffer->Pitch;
    }
}
internal void 
GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset) {

    RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}
