#ifndef __ATLAS_H__
#define __ATLAS_H__

#define ATLAS_WIDTH  506
#define ATLAS_HEIHT  73
#define TEXTURE_ATLAS_FILENAME "atlas.png"
#define ATLAS_FONT_SIZE 32
#define LETTERS_IN_FONT "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890?!&.,_:[]-+"

enum TextureName {
	TextureNone,
	Wall,
};

struct AtlasTexture {
	Vec4 rect;
	// These offsets tell you how much space there is between the rect and the edge of the original document.
	// The atlas is tightly packed, so empty pixels are removed. This can be especially apparent in animations where
	// frames can have different offsets due to different amount of empty pixels around the frames.
	// In many cases you need to add {offset_left, offset_top} to your position. But if you are
	// flipping a texture, then you might need offset_bottom or offset_right.
	f32  offset_top;
	f32  offset_right;
	f32  offset_bottom;
	f32  offset_left;
	Vec2 document_size;
	f32  duration;
};

const AtlasTexture atlas_textures[] = {
	{},
	{ {389.f, 54.f, 16.f, 16.f}, 0.f, 112.f, 112.f, 0.f, {128.f, 128.f}, 0.100f},
};

enum AnimationName {
	AnimNone,
};

enum TagLoopDir {
	Forward,
	Reverse,
	Ping_Pong,
	Ping_Pong_Reverse,
};

// Any aseprite file with frames will create new animations. Also, any tags
// within the aseprite file will make that that into a separate animation.
struct AtlasAnimation {
	TextureName first_frame;
	TextureName last_frame;
	Vec2 document_size;
	TagLoopDir loop_direction;
	u16 repeat;
};

AtlasAnimation atlas_animations[] = {
	{},
};

struct AtlasGlyph {
	Vec4 rect;
	u8 value;
	int offset_x;
	int offset_y;
	int advance_x;
};

const AtlasGlyph atlas_glyphs[] = {
	{{435.f, 28.f, 18.f, 25.f}, 'A', 0, 0, 24},
	{{295.f, 28.f, 18.f, 25.f}, 'B', 0, 0, 24},
	{{235.f, 28.f, 18.f, 25.f}, 'C', 0, 0, 24},
	{{155.f, 28.f, 18.f, 25.f}, 'D', 0, 0, 24},
	{{115.f, 28.f, 18.f, 25.f}, 'E', 0, 0, 24},
	{{406.f, 1.f, 18.f, 25.f}, 'F', 0, 0, 24},
	{{166.f, 1.f, 18.f, 25.f}, 'G', 0, 0, 24},
	{{195.f, 28.f, 18.f, 25.f}, 'H', 0, 0, 24},
	{{55.f, 29.f, 11.f, 25.f}, 'I', 0, 0, 17},
	{{68.f, 29.f, 11.f, 25.f}, 'J', 0, 0, 17},
	{{186.f, 1.f, 18.f, 25.f}, 'K', 0, 0, 24},
	{{206.f, 1.f, 18.f, 25.f}, 'L', 0, 0, 24},
	{{226.f, 1.f, 18.f, 25.f}, 'M', 0, 0, 24},
	{{246.f, 1.f, 18.f, 25.f}, 'N', 0, 0, 24},
	{{266.f, 1.f, 18.f, 25.f}, 'O', 0, 0, 24},
	{{426.f, 1.f, 18.f, 25.f}, 'P', 0, 0, 24},
	{{306.f, 1.f, 18.f, 25.f}, 'Q', 0, 0, 24},
	{{326.f, 1.f, 18.f, 25.f}, 'R', 0, 0, 24},
	{{346.f, 1.f, 18.f, 25.f}, 'S', 0, 0, 24},
	{{366.f, 1.f, 18.f, 25.f}, 'T', 0, 0, 24},
	{{386.f, 1.f, 18.f, 25.f}, 'U', 0, 0, 24},
	{{415.f, 28.f, 18.f, 25.f}, 'V', 0, 0, 24},
	{{115.f, 1.f, 25.f, 25.f}, 'W', 0, 0, 32},
	{{446.f, 1.f, 18.f, 25.f}, 'X', 0, 0, 24},
	{{466.f, 1.f, 18.f, 25.f}, 'Y', 0, 0, 24},
	{{486.f, 1.f, 18.f, 25.f}, 'Z', 0, 0, 24},
	{{327.f, 55.f, 18.f, 18.f}, 'a', 0, 6, 24},
	{{135.f, 28.f, 18.f, 25.f}, 'b', 0, 0, 24},
	{{347.f, 55.f, 18.f, 18.f}, 'c', 0, 6, 24},
	{{175.f, 28.f, 18.f, 25.f}, 'd', 0, 0, 24},
	{{307.f, 55.f, 18.f, 18.f}, 'e', 0, 6, 24},
	{{81.f, 29.f, 11.f, 25.f}, 'f', 0, 0, 17},
	{{35.f, 1.f, 18.f, 26.f}, 'g', 0, 6, 24},
	{{255.f, 28.f, 18.f, 25.f}, 'h', 0, 0, 24},
	{{7.f, 36.f, 4.f, 25.f}, 'i', 0, 0, 10},
	{{1.f, 1.f, 12.f, 33.f}, 'j', -4, 0, 14},
	{{315.f, 28.f, 18.f, 25.f}, 'k', 0, 0, 24},
	{{1.f, 36.f, 4.f, 25.f}, 'l', 0, 0, 10},
	{{140.f, 55.f, 25.f, 18.f}, 'm', 0, 6, 32},
	{{267.f, 55.f, 18.f, 18.f}, 'n', 0, 6, 24},
	{{247.f, 55.f, 18.f, 18.f}, 'o', 0, 6, 24},
	{{75.f, 1.f, 18.f, 26.f}, 'p', 0, 6, 24},
	{{55.f, 1.f, 18.f, 26.f}, 'q', 0, 6, 24},
	{{367.f, 55.f, 11.f, 18.f}, 'r', 0, 6, 17},
	{{187.f, 55.f, 18.f, 18.f}, 's', 0, 6, 24},
	{{94.f, 29.f, 11.f, 25.f}, 't', 0, 0, 17},
	{{167.f, 55.f, 18.f, 18.f}, 'u', 0, 6, 24},
	{{207.f, 55.f, 18.f, 18.f}, 'v', 0, 6, 24},
	{{113.f, 55.f, 25.f, 18.f}, 'w', 0, 6, 32},
	{{227.f, 55.f, 18.f, 18.f}, 'x', 0, 6, 24},
	{{95.f, 1.f, 18.f, 26.f}, 'y', 0, 6, 24},
	{{287.f, 55.f, 18.f, 18.f}, 'z', 0, 6, 24},
	{{495.f, 28.f, 11.f, 25.f}, '1', 0, 0, 17},
	{{35.f, 29.f, 18.f, 25.f}, '2', 0, 0, 24},
	{{286.f, 1.f, 18.f, 25.f}, '3', 0, 0, 24},
	{{215.f, 28.f, 18.f, 25.f}, '4', 0, 0, 24},
	{{275.f, 28.f, 18.f, 25.f}, '5', 0, 0, 24},
	{{335.f, 28.f, 18.f, 25.f}, '6', 0, 0, 24},
	{{355.f, 28.f, 18.f, 25.f}, '7', 0, 0, 24},
	{{475.f, 28.f, 18.f, 25.f}, '8', 0, 0, 24},
	{{455.f, 28.f, 18.f, 25.f}, '9', 0, 0, 24},
	{{395.f, 28.f, 18.f, 25.f}, '0', 0, 0, 24},
	{{375.f, 28.f, 18.f, 25.f}, '?', 0, 0, 24},
	{{107.f, 29.f, 4.f, 25.f}, '!', 0, 0, 10},
	{{142.f, 1.f, 22.f, 25.f}, '&', 0, 0, 28},
	{{428.f, 55.f, 8.f, 8.f}, '.', 0, 16, 14},
	{{407.f, 55.f, 8.f, 12.f}, ',', 0, 16, 14},
	{{438.f, 55.f, 18.f, 5.f}, '_', 0, 27, 24},
	{{380.f, 55.f, 8.f, 18.f}, ':', 0, 6, 14},
	{{15.f, 1.f, 8.f, 33.f}, '[', 0, 0, 14},
	{{25.f, 1.f, 8.f, 33.f}, ']', 0, 0, 14},
	{{458.f, 55.f, 11.f, 5.f}, '-', 0, 9, 17},
	{{13.f, 36.f, 18.f, 19.f}, '+', 0, 2, 24},
};


#endif // __ATLAS_H__
