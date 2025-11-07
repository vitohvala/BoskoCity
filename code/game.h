#pragma once

#include "base.h"
//#include "base.cpp"
#include "atlas.h"

#define HV_RED   (Vec3){1.0f, 0.0f, 0.0f}
#define HV_BLUE  (Vec3){0.0f, 0.0f, 1.0f}
#define HV_GREEN (Vec3){0.0f, 1.0f, 0.0f}
#define HV_BLACK (Vec3){0.0f, 0.0f, 0.0f}

#if OS_LINUX
struct alignas(16) Sprite {
#else
struct  Sprite {
#endif
    Vec2 pos;
    Vec2 size;
    Vec2 aoffset, asize;
    Vec3 color;
};

#define MAX_SPRITES 8192

struct SpriteBatch {
    Sprite *sprite;
    u32 count;
};

struct Entity {
	b32 flip;
	Vec2 pos, vel;
};

struct GameState {
    b32 flip;
    b32 grounded;
    Vec2 pos;
    Vec2 vel;
	Entity player;
};

struct ButtonState {
    b32 ended_down;
    int htransition_count;
};

enum HVJoystick {
    HV_Keyboard,
    HV_Joystick1,
    HV_Joystick2,
    HV_Joystick3,
    HV_Joystick_Count,
};

struct ControllerInput {
    b32 is_analog;
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
};

struct GameInput {
    ControllerInput cinput[HV_Joystick_Count];
};

struct Memory {
    b32 is_init;
    Arena *temp;
    Arena *transient;
    Arena *permanent;

    Console *log;
    SpriteBatch *sb;
    SpriteBatch *circle_batch;

    GameState *state;
    GameInput *input;

    Vec2 screen_size;

    f32 dt;
};

#define UPDATE_FUNC(name) void name(Memory *m)
typedef UPDATE_FUNC(UpdateP);

extern "C" {
    UPDATE_FUNC(game_update);
}

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
