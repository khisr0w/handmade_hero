/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  4/18/2024 10:31:38 PM                                         |
    |    Last Modified:                                                                |
    |                                                                                  |
    +======================================| Copyright © Sayed Abid Hashimi |==========+  */

#if !defined(HANDMADE_ASSET_H)

enum asset_state
{
	AssetState_Unloaded,
	AssetState_Queued,
	AssetState_Loaded,
    AssetState_Locked,
};

struct loaded_sound {
    i32 SampleCount;
    void *Memory;
};

struct asset_slot {
	asset_state State;
    union {
	    loaded_bitmap *Bitmap;
	    loaded_sound *Sound;
    };
};

enum asset_tag_id {

    Tag_Smoothness,
    Tag_Flatness,
    Tag_FacingDirection, /* Angle in radiants off of due right. */

    Tag_Count,
};

enum asset_type_id {
    Asset_None,

    Asset_Shadow,
    Asset_Tree,
    Asset_Sword,
    Asset_Rock,

    Asset_Grass,
    Asset_Tuft,
    Asset_Stone,

    Asset_Head,
    Asset_Cape,
    Asset_Torso,

	Asset_Count,
};

struct asset_tag {
    uint32 ID; /* Tag ID */
    real32 Value;
};

struct asset_type {
    uint32 FirstAssetIndex;
    uint32 OnePastLastAssetIndex;
};

struct asset {
    uint32 FirstTagIndex;
    uint32 OnePastLastTagIndex;

    uint32 SlotID;
};

struct asset_vector {
    f32 E[Tag_Count];
};

struct asset_bitmap_info {
    char *FileName;
    v2 AlignPercentage;
};
struct asset_sound_info {
    char *FileName;
};

struct game_assets {
	// TODO(Abid): Not thrilled about this back-pointer
	struct transient_state *TranState;
	memory_arena Arena;

    f32 TagRange[Tag_Count];

    uint32 BitmapCount;
    asset_bitmap_info *BitmapInfos;
    asset_slot *Bitmaps;

    uint32 SoundCount;
    asset_sound_info *SoundInfos;
    asset_slot *Sounds;

    uint32 AssetCount;
    asset *Assets;

    uint32 TagCount;
    asset_tag *Tags;

    asset_type AssetTypes[Asset_Count];

    
	// NOTE(Abid) Structured assets
    // hero_bitmaps HeroBitmaps[4];

    /* TODO(Abid): Only used here for debugging and must be removed once pak file is in place */
    /* NOTE(Abid): Hacky way to turn our asset allocation into a state machine */
    uint32 DEBUGUsedBitmapCount;
    uint32 DEBUGUsedAssetCount;
    uint32 DEBUGUsedTagCount;
    asset_type *DEBUGAssetType;
    asset *DEBUGAsset;
};

struct bitmap_id { uint32 Value; };
struct sound_id { uint32 Value; };

inline loaded_bitmap *
GetBitmap(game_assets *Assets, bitmap_id ID) {
	loaded_bitmap *Result = Assets->Bitmaps[ID.Value].Bitmap;

	return Result;
}

internal void LoadBitmap(game_assets *Assets, bitmap_id ID);
internal void LoadSound(game_assets *Assets, sound_id ID);

#define HANDMADE_ASSET_H
#endif
