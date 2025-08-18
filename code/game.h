#pragma once

#include "base.h"
//#include "base.cpp"

#define HV_RED   (Vec3){1.0f, 0.0f, 0.0f}
#define HV_BLUE  (Vec3){0.0f, 0.0f, 1.0f}
#define HV_GREEN (Vec3){0.0f, 1.0f, 0.0f}



struct Sprite {
    Vec2 pos;
    Vec2 size;
    Vec2 aoffset, asize;
    Vec3 color;
};

#define MAX_SPRITES 8192 * 2

struct Glyph {
	Vec4 rect;
	u8 value;
	int offset_x;
	int offset_y;
	int advance_x;
};

struct Texture{
	Vec4 rect;
	// These offsets tell you how much space there is between the rect and the edge of the original document.
	// The atlas is tightly packed, so empty pixels are removed. This can be especially apparent in animations where
	// frames can have different offsets due to different amount of empty pixels around the frames.
	// In many cases you need to add {offset_left, offset_top} to your position. But if you are
	// flipping a texture, then you might need offset_bottom or offset_right.
	f32 offset_top;
	f32 offset_right;
	f32 offset_bottom;
	f32 offset_left;
	Vec2 document_size;
	f32 duration;
};

enum TextureName {
	Body_Template0,
	Body_Template1,
	Body_Template2,
	Body_Template3,
	Body_Template4,
	Body_Template5,
	Body_Template6,
	Body_Template7,
	Body_Template8,
	Body_Template9,
	Body_Template10,
	Body_Template11,
	Body_Template12,
	Body_Template13,
	Body_Template14,
	Body_Template15,
	Body_Template16,
	Body_Template17,
	Body_Template18,
	Body_Template19,
	Body_Template20,
	Body_Template21,
	Body_Template22,
	Body_Template23,
	Body_Template24,
	Body_Template25,
	Body_Template26,
	Body_Template27,
	Body_Template28,
	Body_Template29,
	Body_Template30,
	Body_Template31,
	Body_Template32,
	Body_Template33,
	Body_Template34,
	Body_Template35,
	Body_Template36,
	Body_Template37,
	Body_Template38,
	Body_Template39,
	Body_Template40,
	Body_Template41,
	Body_Template42,
	Body_Template43,
	Body_Template44,
	Body_Template45,
	Body_Template46,
	Body_Template47,
	Body_Template48,
	Body_Template49,
};

struct Animation {
	TextureName first_frame;
	TextureName last_frame;
};

enum AnimationName {
	Body_Template_Walk,
	Body_Template_Flat_Jump,
	Body_Template_Run,
	Body_Template_Idle,
	Body_Template_Jump,
};

struct SpriteBatch {
    Sprite *sprite;
    u32 count;
};

struct GameState {
    Vec2 pos;
    Vec2 vel;
    Animation animation;
    f32 animatet;
    usize animation_index;
    usize animate_index;
};



struct ButtonState {
    int htransition_count;
    b32 ended_down;
};

enum HVJoystick {
    HV_Keyboard,
    HV_Joystick1,
    HV_Joystick2,
    HV_Joystick3,
    HV_Joystick_Count,
};


struct ControllerInput {
    union {
        ButtonState buttons[14];
        struct {
            ButtonState move_up;
            ButtonState move_down;
            ButtonState move_right;
            ButtonState move_left;
            ButtonState action_up;
            ButtonState action_down;
            ButtonState action_left;
            ButtonState action_right;

            ButtonState start;
            ButtonState back;

            ButtonState l1trigger;
            ButtonState r1trigger;
            ButtonState l2trigger;
            ButtonState r2trigger;
        };
    };

    Vec2 avg;

    b32 is_analog;
};

struct GameInput {
    ControllerInput cinput[HV_Joystick_Count];
};

struct Memory {
    Arena *temp;
    Arena *transient;
    Arena *permanent;

    Console *log;
    SpriteBatch *sb;

    GameState *state;
    GameInput *input;

    f32 dt;
    b32 is_init;
};

//#include "gl_renderer.h"
//#include "gl_renderer.cpp"

#define UPDATE_FUNC(name) void name(Memory *m)
typedef UPDATE_FUNC(UpdateP);


/*
Glyph glyphs[] = {
	{ {35.f,  29.f, 18.f, 25.f}, 'A',  0,  0,  24},
	{ {415.f, 28.f, 18.f, 25.f}, 'B',  0,  0,  24},
	{ {295.f, 28.f, 18.f, 25.f}, 'C',  0,  0,  24},
	{ {235.f, 28.f, 18.f, 25.f}, 'D',  0,  0,  24},
	{ {406.f, 1.f,  18.f, 25.f}, 'E',  0,  0,  24},
	{ {166.f, 1.f,  18.f, 25.f}, 'F',  0,  0,  24},
	{ {286.f, 1.f,  18.f, 25.f}, 'G',  0,  0,  24},
	{ {115.f, 28.f, 18.f, 25.f}, 'H',  0,  0,  24},
	{ {495.f, 28.f, 11.f, 25.f}, 'I',  0,  0,  17},
	{ {94.f,  29.f, 11.f, 25.f}, 'J',  0,  0,  17},
	{ {186.f, 1.f,  18.f, 25.f}, 'K',  0,  0,  24},
	{ {206.f, 1.f,  18.f, 25.f}, 'L',  0,  0,  24},
	{ {226.f, 1.f,  18.f, 25.f}, 'M',  0,  0,  24},
	{ {246.f, 1.f,  18.f, 25.f}, 'N',  0,  0,  24},
	{ {266.f, 1.f,  18.f, 25.f}, 'O',  0,  0,  24},
	{ {426.f, 1.f,  18.f, 25.f}, 'P',  0,  0,  24},
	{ {306.f, 1.f,  18.f, 25.f}, 'Q',  0,  0,  24},
	{ {326.f, 1.f,  18.f, 25.f}, 'R',  0,  0,  24},
	{ {346.f, 1.f,  18.f, 25.f}, 'S',  0,  0,  24},
	{ {366.f, 1.f,  18.f, 25.f}, 'T',  0,  0,  24},
	{ {386.f, 1.f,  18.f, 25.f}, 'U',  0,  0,  24},
	{ {435.f, 28.f, 18.f, 25.f}, 'V',  0,  0,  24},
	{ {115.f, 1.f,  25.f, 25.f}, 'W',  0,  0,  32},
	{ {446.f, 1.f,  18.f, 25.f}, 'X',  0,  0,  24},
	{ {466.f, 1.f,  18.f, 25.f}, 'Y',  0,  0,  24},
	{ {486.f, 1.f,  18.f, 25.f}, 'Z',  0,  0,  24},
	{ {307.f, 55.f, 18.f, 18.f}, 'a',  0,  6,  24},
	{ {135.f, 28.f, 18.f, 25.f}, 'b',  0,  0,  24},
	{ {267.f, 55.f, 18.f, 18.f}, 'c',  0,  6,  24},
	{ {175.f, 28.f, 18.f, 25.f}, 'd',  0,  0,  24},
	{ {247.f, 55.f, 18.f, 18.f}, 'e',  0,  6,  24},
	{ {68.f,  29.f, 11.f, 25.f}, 'f',  0,  0,  17},
	{ {55.f,  1.f,  18.f, 26.f}, 'g',  0,  6,  24},
	{ {255.f, 28.f, 18.f, 25.f}, 'h',  0,  0,  24},
	{ {1.f,   36.f, 4.f,  25.f}, 'i',  0,  0,  10},
	{ {1.f,   1.f,  12.f, 33.f}, 'j',  -4, 0,  14},
	{ {315.f, 28.f, 18.f, 25.f}, 'k',  0,  0,  24},
	{ {7.f,   36.f, 4.f,  25.f}, 'l',  0,  0,  10},
	{ {140.f, 55.f, 25.f, 18.f}, 'm',  0,  6,  32},
	{ {167.f, 55.f, 18.f, 18.f}, 'n',  0,  6,  24},
	{ {187.f, 55.f, 18.f, 18.f}, 'o',  0,  6,  24},
	{ {75.f,  1.f,  18.f, 26.f}, 'p',  0,  6,  24},
	{ {35.f,  1.f,  18.f, 26.f}, 'q',  0,  6,  24},
	{ {69.f,  56.f, 11.f, 18.f}, 'r',  0,  6,  17},
	{ {227.f, 55.f, 18.f, 18.f}, 's',  0,  6,  24},
	{ {81.f,  29.f, 11.f, 25.f}, 't',  0,  0,  17},
	{ {347.f, 55.f, 18.f, 18.f}, 'u',  0,  6,  24},
	{ {207.f, 55.f, 18.f, 18.f}, 'v',  0,  6,  24},
	{ {113.f, 55.f, 25.f, 18.f}, 'w',  0,  6,  32},
	{ {327.f, 55.f, 18.f, 18.f}, 'x',  0,  6,  24},
	{ {95.f,  1.f,  18.f, 26.f}, 'y',  0,  6,  24},
	{ {287.f, 55.f, 18.f, 18.f}, 'z',  0,  6,  24},
	{ {55.f,  29.f, 11.f, 25.f}, '1',  0,  0,  17},
	{ {475.f, 28.f, 18.f, 25.f}, '2',  0,  0,  24},
	{ {455.f, 28.f, 18.f, 25.f}, '3',  0,  0,  24},
	{ {395.f, 28.f, 18.f, 25.f}, '4',  0,  0,  24},
	{ {375.f, 28.f, 18.f, 25.f}, '5',  0,  0,  24},
	{ {355.f, 28.f, 18.f, 25.f}, '6',  0,  0,  24},
	{ {335.f, 28.f, 18.f, 25.f}, '7',  0,  0,  24},
	{ {275.f, 28.f, 18.f, 25.f}, '8',  0,  0,  24},
	{ {215.f, 28.f, 18.f, 25.f}, '9',  0,  0,  24},
	{ {195.f, 28.f, 18.f, 25.f}, '0',  0,  0,  24},
	{ {155.f, 28.f, 18.f, 25.f}, '?',  0,  0,  24},
	{ {107.f, 29.f, 4.f,  25.f}, '!',  0,  0,  10},
	{ {142.f, 1.f,  22.f, 25.f}, '&',  0,  0,  28},
	{ {295.f, 96.f, 6.f,  4.f }, '.',  0,  16, 14},
	{ {274.f, 94.f, 8.f,  12.f}, ',',  0,  16, 14},
	{ {305.f, 94.f, 18.f, 5.f }, '_',  0,  27, 24},
	{ {82.f,  56.f, 8.f,  18.f}, ':',  0,  6,  14},
	{ {15.f,  1.f,  8.f,  33.f}, '[',  0,  0,  14},
	{ {25.f,  1.f,  8.f,  33.f}, ']',  0,  0,  14},
	{ {325.f, 94.f, 11.f, 5.f }, '-',  0,  9,  17},
	{ {13.f,  36.f, 18.f, 19.f}, '+',  0,  2,  24},
}; */

Glyph glyphs[] =  {
	{ {386.f, 94.f, 4.f,  4.f }, ' ',  0,  0,  10},  //32  0
	{ {107.f, 29.f, 4.f,  25.f}, '!',  0,  0,  10},   //33  1
	{ {155.f, 28.f, 18.f, 25.f}, '?',  0,  0,  24},  //34  2
	{ {155.f, 28.f, 18.f, 25.f}, '?',  0,  0,  24},  //35  3
	{ {155.f, 28.f, 18.f, 25.f}, '?',  0,  0,  24},  //36  4
	{ {155.f, 28.f, 18.f, 25.f}, '?',  0,  0,  24},  //37  5
	{ {142.f, 1.f,  22.f, 25.f}, '&',  0,  0,  28},  //38  6
	{ {274.f, 94.f, 8.f,  12.f}, ',',  0,  16, 14},  //39  7
	{ {15.f,  1.f,  8.f,  33.f}, '[',  0,  0,  14},  //40  8
	{ {25.f,  1.f,  8.f,  33.f}, ']',  0,  0,  14},  //41  9
	{ {13.f,  36.f, 18.f, 19.f}, '+',  0,  2,  24},  //42  0
	{ {13.f,  36.f, 18.f, 19.f}, '+',  0,  2,  24},  //43  10
	{ {274.f, 94.f, 8.f,  12.f}, ',',  0,  16, 14},  //44  11
	{ {325.f, 94.f, 11.f, 5.f }, '-',  0,  9,  17},  //45  12
	{ {295.f, 94.f, 8.f,  8.f }, '.',  0,  16, 14},  //46  13
	{ {107.f, 29.f, 4.f,  25.f}, ' ',  0,  0,  10},  //47  14
	{ {195.f, 28.f, 18.f, 25.f}, '0',  0,  0,  24},  //48  15
	{ {55.f,  29.f, 11.f, 25.f}, '1',  0,  0,  17},  //49  16
	{ {475.f, 28.f, 18.f, 25.f}, '2',  0,  0,  24},  //50  17
	{ {455.f, 28.f, 18.f, 25.f}, '3',  0,  0,  24},  //51  18
	{ {395.f, 28.f, 18.f, 25.f}, '4',  0,  0,  24},  //52  19
	{ {375.f, 28.f, 18.f, 25.f}, '5',  0,  0,  24},  //53  20
	{ {355.f, 28.f, 18.f, 25.f}, '6',  0,  0,  24},  //54  21
	{ {335.f, 28.f, 18.f, 25.f}, '7',  0,  0,  24},  //55  22
	{ {275.f, 28.f, 18.f, 25.f}, '8',  0,  0,  24},  //56  23
	{ {215.f, 28.f, 18.f, 25.f}, '9',  0,  0,  24},  //57  24
	{ {82.f,  56.f, 8.f,  18.f}, ':',  0,  6,  14},  //58  25
	{ {82.f,  56.f, 8.f,  18.f}, ':',  0,  6,  14},  //59  26
	{ {82.f,  56.f, 8.f,  18.f}, ':',  0,  6,  14},  //60  27
	{ {82.f,  56.f, 8.f,  18.f}, ':',  0,  6,  14},  //61  28
	{ {82.f,  56.f, 8.f,  18.f}, ':',  0,  6,  14},  //62  29
	{ {155.f, 28.f, 18.f, 25.f}, '?',  0,  0,  24},  //63  30
	{ {155.f, 28.f, 18.f, 25.f}, '?',  0,  0,  24},  //64  31
	{ {35.f,  29.f, 18.f, 25.f}, 'A',  0,  0,  24},  //65  32
	{ {415.f, 28.f, 18.f, 25.f}, 'B',  0,  0,  24},  //66  33
	{ {295.f, 28.f, 18.f, 25.f}, 'C',  0,  0,  24},  //67
	{ {235.f, 28.f, 18.f, 25.f}, 'D',  0,  0,  24},  //68
	{ {406.f, 1.f,  18.f, 25.f}, 'E',  0,  0,  24},  //69
	{ {166.f, 1.f,  18.f, 25.f}, 'F',  0,  0,  24},  //70
	{ {286.f, 1.f,  18.f, 25.f}, 'G',  0,  0,  24},  //71
	{ {115.f, 28.f, 18.f, 25.f}, 'H',  0,  0,  24},  //72
	{ {495.f, 28.f, 11.f, 25.f}, 'I',  0,  0,  17},  //73
	{ {94.f,  29.f, 11.f, 25.f}, 'J',  0,  0,  17},  //74
	{ {186.f, 1.f,  18.f, 25.f}, 'K',  0,  0,  24},  //75
	{ {206.f, 1.f,  18.f, 25.f}, 'L',  0,  0,  24},  //76
	{ {226.f, 1.f,  18.f, 25.f}, 'M',  0,  0,  24},  //77
	{ {246.f, 1.f,  18.f, 25.f}, 'N',  0,  0,  24},  //78
	{ {266.f, 1.f,  18.f, 25.f}, 'O',  0,  0,  24},  //79
	{ {426.f, 1.f,  18.f, 25.f}, 'P',  0,  0,  24},  //80
	{ {306.f, 1.f,  18.f, 25.f}, 'Q',  0,  0,  24},  //81
	{ {326.f, 1.f,  18.f, 25.f}, 'R',  0,  0,  24},  //82
	{ {346.f, 1.f,  18.f, 25.f}, 'S',  0,  0,  24},  //83
	{ {366.f, 1.f,  18.f, 25.f}, 'T',  0,  0,  24},  //84
	{ {386.f, 1.f,  18.f, 25.f}, 'U',  0,  0,  24},  //85
	{ {435.f, 28.f, 18.f, 25.f}, 'V',  0,  0,  24},  //86
	{ {115.f, 1.f,  25.f, 25.f}, 'W',  0,  0,  32},  //87
	{ {446.f, 1.f,  18.f, 25.f}, 'X',  0,  0,  24},  //88
	{ {466.f, 1.f,  18.f, 25.f}, 'Y',  0,  0,  24},  //89
	{ {486.f, 1.f,  18.f, 25.f}, 'Z',  0,  0,  24},  //90
	{ {15.f,  1.f,  8.f,  33.f}, '[',  0,  0,  14},  //91
	{ {25.f,  1.f,  8.f,  33.f}, ']',  0,  0,  14},  //92
	{ {25.f,  1.f,  8.f,  33.f}, ']',  0,  0,  14},  //93
	{ {305.f, 94.f, 18.f, 5.f }, '_',  0,  27, 24},  //94
	{ {305.f, 94.f, 18.f, 5.f }, '_',  0,  27, 24},  //95
	{ {305.f, 94.f, 18.f, 5.f }, '_',  0,  27, 24},  //96
	{ {307.f, 55.f, 18.f, 18.f}, 'a',  0,  6,  24},
	{ {135.f, 28.f, 18.f, 25.f}, 'b',  0,  0,  24},
	{ {267.f, 55.f, 18.f, 18.f}, 'c',  0,  6,  24},
	{ {175.f, 28.f, 18.f, 25.f}, 'd',  0,  0,  24},
	{ {247.f, 55.f, 18.f, 18.f}, 'e',  0,  6,  24},
	{ {68.f,  29.f, 11.f, 25.f}, 'f',  0,  0,  17},
	{ {55.f,  1.f,  18.f, 26.f}, 'g',  0,  6,  24},
	{ {255.f, 28.f, 18.f, 25.f}, 'h',  0,  0,  24},
	{ {1.f,   36.f, 4.f,  25.f}, 'i',  0,  0,  10},
	{ {1.f,   1.f,  12.f, 33.f}, 'j',  -4, 0,  14},
	{ {315.f, 28.f, 18.f, 25.f}, 'k',  0,  0,  24},
	{ {7.f,   36.f, 4.f,  25.f}, 'l',  0,  0,  10},
	{ {140.f, 55.f, 25.f, 18.f}, 'm',  0,  6,  32},
	{ {167.f, 55.f, 18.f, 18.f}, 'n',  0,  6,  24},
	{ {187.f, 55.f, 18.f, 18.f}, 'o',  0,  6,  24},
	{ {75.f,  1.f,  18.f, 26.f}, 'p',  0,  6,  24},
	{ {35.f,  1.f,  18.f, 26.f}, 'q',  0,  6,  24},
	{ {69.f,  56.f, 11.f, 18.f}, 'r',  0,  6,  17},
	{ {227.f, 55.f, 18.f, 18.f}, 's',  0,  6,  24},
	{ {81.f,  29.f, 11.f, 25.f}, 't',  0,  0,  17},
	{ {347.f, 55.f, 18.f, 18.f}, 'u',  0,  6,  24},
	{ {207.f, 55.f, 18.f, 18.f}, 'v',  0,  6,  24},
	{ {113.f, 55.f, 25.f, 18.f}, 'w',  0,  6,  32},
	{ {327.f, 55.f, 18.f, 18.f}, 'x',  0,  6,  24},
	{ {95.f,  1.f,  18.f, 26.f}, 'y',  0,  6,  24},
	{ {287.f, 55.f, 18.f, 18.f}, 'z',  0,  6,  24},
};

Texture textures[] = {
	{ {0.f,   62.f, 27.f, 18.f}, 46.f, 13.f, 0.f,  24.f, {64.f, 64.f}, 0.100f},
	{ {118.f, 74.f, 25.f, 18.f}, 46.f, 15.f, 0.f,  24.f, {64.f, 64.f}, 0.100f},
	{ {144.f, 74.f, 23.f, 18.f}, 46.f, 16.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {479.f, 74.f, 22.f, 17.f}, 47.f, 17.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {50.f,  75.f, 21.f, 17.f}, 47.f, 18.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {22.f,  93.f, 19.f, 17.f}, 47.f, 20.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {405.f, 74.f, 17.f, 18.f}, 46.f, 22.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {331.f, 74.f, 18.f, 18.f}, 46.f, 21.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {232.f, 74.f, 19.f, 18.f}, 46.f, 20.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {465.f, 92.f, 20.f, 17.f}, 47.f, 19.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {0.f,   81.f, 21.f, 17.f}, 47.f, 18.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {455.f, 74.f, 23.f, 17.f}, 47.f, 16.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {292.f, 74.f, 19.f, 18.f}, 46.f, 20.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {72.f,  75.f, 17.f, 17.f}, 47.f, 22.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {178.f, 93.f, 17.f, 16.f}, 48.f, 24.f, 0.f,  23.f, {64.f, 64.f}, 0.100f},
	{ {312.f, 74.f, 18.f, 18.f}, 46.f, 21.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {385.f, 54.f, 18.f, 19.f}, 45.f, 21.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {458.f, 54.f, 17.f, 19.f}, 45.f, 22.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {476.f, 54.f, 17.f, 19.f}, 45.f, 24.f, 0.f,  23.f, {64.f, 64.f}, 0.100f},
	{ {494.f, 54.f, 17.f, 19.f}, 45.f, 25.f, 0.f,  22.f, {64.f, 64.f}, 0.100f},
	{ {32.f,  55.f, 17.f, 19.f}, 45.f, 25.f, 0.f,  22.f, {64.f, 64.f}, 0.100f},
	{ {369.f, 74.f, 17.f, 18.f}, 46.f, 25.f, 0.f,  22.f, {64.f, 64.f}, 0.100f},
	{ {196.f, 93.f, 17.f, 16.f}, 48.f, 24.f, 0.f,  23.f, {64.f, 64.f}, 0.100f},
	{ {212.f, 74.f, 19.f, 18.f}, 46.f, 20.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {423.f, 74.f, 31.f, 17.f}, 44.f, 12.f, 3.f,  21.f, {64.f, 64.f}, 0.100f},
	{ {80.f,  93.f, 32.f, 16.f}, 45.f, 11.f, 3.f,  21.f, {64.f, 64.f}, 0.100f},
	{ {91.f,  74.f, 26.f, 18.f}, 46.f, 17.f, 0.f,  21.f, {64.f, 64.f}, 0.100f},
	{ {28.f,  75.f, 21.f, 17.f}, 47.f, 18.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {423.f, 92.f, 20.f, 17.f}, 44.f, 23.f, 3.f,  21.f, {64.f, 64.f}, 0.100f},
	{ {158.f, 93.f, 19.f, 16.f}, 45.f, 24.f, 3.f,  21.f, {64.f, 64.f}, 0.100f},
	{ {168.f, 74.f, 22.f, 18.f}, 46.f, 21.f, 0.f,  21.f, {64.f, 64.f}, 0.100f},
	{ {42.f,  93.f, 19.f, 17.f}, 47.f, 20.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {191.f, 74.f, 20.f, 18.f}, 46.f, 19.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {444.f, 92.f, 20.f, 17.f}, 47.f, 19.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {136.f, 93.f, 21.f, 16.f}, 48.f, 19.f, 0.f,  24.f, {64.f, 64.f}, 0.100f},
	{ {250.f, 93.f, 22.f, 15.f}, 49.f, 18.f, 0.f,  24.f, {64.f, 64.f}, 0.100f},
	{ {113.f, 93.f, 22.f, 16.f}, 48.f, 18.f, 0.f,  24.f, {64.f, 64.f}, 0.100f},
	{ {486.f, 92.f, 20.f, 17.f}, 47.f, 19.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {272.f, 74.f, 19.f, 18.f}, 46.f, 20.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {62.f,  93.f, 17.f, 17.f}, 47.f, 22.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {232.f, 93.f, 17.f, 16.f}, 48.f, 24.f, 0.f,  23.f, {64.f, 64.f}, 0.100f},
	{ {350.f, 74.f, 18.f, 18.f}, 46.f, 21.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
	{ {366.f, 54.f, 18.f, 19.f}, 29.f, 21.f, 16.f, 25.f, {64.f, 64.f}, 0.100f},
	{ {50.f,  55.f, 17.f, 19.f}, 25.f, 22.f, 20.f, 25.f, {64.f, 64.f}, 0.100f},
	{ {440.f, 54.f, 17.f, 19.f}, 23.f, 24.f, 22.f, 23.f, {64.f, 64.f}, 0.100f},
	{ {404.f, 54.f, 17.f, 19.f}, 25.f, 25.f, 20.f, 22.f, {64.f, 64.f}, 0.100f},
	{ {422.f, 54.f, 17.f, 19.f}, 29.f, 25.f, 16.f, 22.f, {64.f, 64.f}, 0.100f},
	{ {387.f, 74.f, 17.f, 18.f}, 46.f, 25.f, 0.f,  22.f, {64.f, 64.f}, 0.100f},
	{ {214.f, 93.f, 17.f, 16.f}, 48.f, 24.f, 0.f,  23.f, {64.f, 64.f}, 0.100f},
	{ {252.f, 74.f, 19.f, 18.f}, 46.f, 20.f, 0.f,  25.f, {64.f, 64.f}, 0.100f},
};

Animation animations[] =  {
	{ Body_Template0,  Body_Template11 },
	{ Body_Template12, Body_Template23 },
	{ Body_Template24, Body_Template31 },
	{ Body_Template32, Body_Template37 },
	{ Body_Template38, Body_Template49 },
};

function b32
was_pressed(ButtonState *state) {
	b32 result = ((state->htransition_count > 1) ||
	             ((state->htransition_count == 1) &&
	             ( state->ended_down)));
	return result;
}

function void
process_keyboard_message(ButtonState *new_state, b32 is_down) {
    if (new_state->ended_down != is_down) {
        new_state->ended_down = is_down;
        new_state->htransition_count++;
    }
}