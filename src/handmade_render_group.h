/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  1/4/2021 7:30:49 AM                                           |
    |    Last Modified:                                                                |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright © All rights reserved |======+  */

#if !defined(HANDMADE_RENDER_GROUP_H)

/* 
   NOTE(Khisrow):

   1) Everywhere outside the renderer, Y _always_ goes upward, X is to the right.

   2) All bitmap, including the render target, are assumed to be bottom-up (meaning
	  the first row pointer points to the the bottom-most row when viewed on screen).

   3) Unless otherwise specified, all inputs to the renderer are in world coordinates
   	  ("meters"), _not_ pixels. Anything that is in pixel values will be explicitly
	  marked as such.

   4) Z is a special coordinate because it is broken into discrete slices. And the
	  renderer actually understand these slices (potentially).

	// TODO(Khisrow): ZHANDLING
*/

struct loaded_bitmap
{
	int32_t Width;
	int32_t Height;
	int32_t Pitch;
	void *Memory;
};

struct environment_map
{
	loaded_bitmap LOD[4];
	real32 Pz;
};

struct render_basis
{
	v3 P;
};

struct render_entity_basis
{
	render_basis *Basis;
	v2 Offset;
	real32 OffsetZ;
	real32 EntityZC;
};

enum render_group_entry_type
{
	RenderGroupEntryType_render_entry_clear,
	RenderGroupEntryType_render_entry_bitmap,
	RenderGroupEntryType_render_entry_rectangle,
	RenderGroupEntryType_render_entry_coordinate_system,
	RenderGroupEntryType_render_entry_saturation,
};

struct render_group_entry_header
{
	render_group_entry_type Type;
};

struct render_entry_clear
{
	v4 Color;
};

struct render_entry_saturation
{
	real32 Level;
};

struct render_entry_bitmap
{
	render_entity_basis EntityBasis;
	loaded_bitmap *Bitmap;
	v4 Color;
};

struct render_entry_rectangle
{
	render_entity_basis EntityBasis;
	v4 Color;
	v2 Dim;
};

// NOTE(Khisrow): This is only for test:
// {
struct render_entry_coordinate_system
{
	v2 Origin;
	v2 XAxis;
	v2 YAxis;
	v4 Color;
	loaded_bitmap *Texture;
	loaded_bitmap *NormalMap;

	environment_map *Top;
	environment_map *Middle;
	environment_map *Bottom;
};
// }

struct render_group
{
	render_basis *DefaultBasis;
	real32 MetersToPixels;

	uint32_t MaxPushBufferSize;
	uint32_t PushBufferSize;
	uint8_t *PushBufferBase;
};

#define HANDMADE_RENDER_GROUP_H
#endif
