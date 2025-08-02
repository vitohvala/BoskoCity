#pragma once

#include "base.h"
//#include "base.cpp"

struct Memory {
    Arena temp;
    Arena transient;
    Arena permanent;

    Console log;

    b32 is_init;
};

//#include "gl_renderer.h"
//#include "gl_renderer.cpp"

#define UPDATE_FUNC(name) void name(Memory *m)
typedef UPDATE_FUNC(UpdateP);
