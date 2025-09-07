#pragma once

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#if defined __clang__
#define COMPILER_CLANG 1
// clang is defining _MSC_VER
#if defined __linux__
#define OS_LINUX 1
#elif defined __APPLE__
#define OS_MAC 1
#elif defined _WIN32
#define OS_WINDOWS 1
#else
#error missing OS detection
#endif

#elif defined _MSC_VER
#define COMPILER_CL 1

#if defined _WIN32
#define OS_WINDOWS 1
#else
#error missing OS detection
#endif

#elif defined __GNUC__
#define COMPILER_GCC 1

#if defined __linux__
#define OS_LINUX 1
#elif defined __APPLE__
#define OS_MAC 1
#elif defined _WIN32
#define OS_WINDOWS 1
#else
#error missing OS detection
#endif

#elif defined __MINGW32__
#define COMPILER_MINGW32 1

#if defined __linux__
#define OS_LINUX 1
#elif defined __APPLE__
#define OS_MAC 1
#elif defined _WIN32
#define OS_WINDOWS 1
#else
#error missing OS detection
#endif

#elif defined __MINGW64__
#define COMPILER_MINGW64 1

#if defined __linux__
#define OS_LINUX 1
#elif defined __APPLE__
#define OS_MAC 1
#elif defined _WIN32
#define OS_WINDOWS 1
#else
#error missing OS detection
#endif

#endif

#if !defined COMPILER_CL
#define COMPILER_CL 0
#endif

#if !defined COMPILER_GCC
#define COMPILER_GCC 0
#endif

#if !defined COMPILER_CLANG
#define COMPILER_CLANG 0
#endif

#if !defined COMPILER_MINGW32
#define COMPILER_MINGW32 0
#endif

#if !defined COMPILER_MINGW64
#define COMPILER_MINGW64 0
#endif

#if !defined OS_WINDOWS
#define OS_WINDOWS 0
#endif

#if !defined OS_LINUX
#define OS_LINUX 0
#endif

#if !defined OS_MAC
#define OS_MAC 0
#endif
#if defined(__cplusplus)
# define LANG_CPP 1
#else
# define LANG_C 1
#endif

#if !defined(LANG_CPP)
# define LANG_CPP 0
#endif
#if !defined(LANG_C)
# define LANG_C 0
#endif

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define CLAMP(A, X, B) (((X) < (A)) ? (A) : ((X) > (B)) ? (B) : (X))

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t  i8;

typedef size_t  usize;

typedef double  f64;
typedef float   f32;

typedef uintptr_t uptr;
typedef unsigned int uint;

typedef u32     b32;

#define Kilobyte 1024
#define Megabyte Kilobyte * 1024
#define Gigabyte Megabyte * 1024


#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((u64)(x) << 30llu)
#define TB(x) ((u64)(x) << 40llu)

#define Thousand(x) ((x)*1000)
#define Million(x)  ((x)*1000000llu)
#define Billion(x)  ((x)*1000000000llu)
#define Trillion(x) ((x)*1000000000000llu)

#define nil (NULL)

#define Statement(x) do { x } while(0)

#ifdef DEBUG_BUILD
    #if defined COMPILER_CL
        #define Trap __debugbreak()
    #elif COMPILER_GCC || COMPILER_CLANG
        #define Trap __builtin_trap()
    #else
        #define Trap Statement( if(!(x)) { *(int*) 0 = 0; } )
    #endif

    #define Assert(expr) Statement( if(!(expr)) { Trap; })
#else
    #define Assert(x) (void)(x)
#endif

union Vec2{
    struct {
        f32 x, y;
    };
    f32 v[2];
};

union Vec3{
    struct {
        f32 x, y, z;
    };
    f32 v[3];
};

union Vec4{
    struct {
        f32 x, y, z, w;
    };
    f32 v[4];
};

union Mat4 {
    struct {
        Vec4 m0;
        Vec4 m1;
        Vec4 m2;
        Vec4 m3;
    };
    Vec4 m[4];
};

#define function static
#define local static
#define global static

function Vec2 operator +(Vec2 l,  Vec2 r);
function Vec2 operator+=(Vec2 &l, Vec2 r);
function Vec2 operator -(Vec2 l,  Vec2 r);
function Vec2 operator-=(Vec2 &l, Vec2 r);
function Vec2 operator -(Vec2 l);
function Vec2 operator *(Vec2 l,  Vec2 r);
function Vec2 operator *(Vec2 l,  f32  r);
function Vec2 operator*=(Vec2 &l, Vec2 r);
function b32  operator== (Vec2 l, Vec2 r);
function b32  operator!= (Vec2 l, Vec2 r);

function Vec3 operator +(Vec3 l,  Vec3 r);
function Vec3 operator+=(Vec3 &l, Vec3 r);
function Vec3 operator -(Vec3 l,  Vec3 r);
function Vec3 operator-=(Vec3 &l, Vec3 r);
function Vec3 operator -(Vec3 l);
function Vec3 operator *(Vec3 l,  Vec3 r);
function Vec3 operator *(Vec3 l,  f32  r);
function Vec3 operator*=(Vec3 &l, Vec3 r);
function Vec3 operator*=(Vec3 &l, f32 r);
function b32  operator== (Vec3 l, Vec3 r);
function b32  operator!= (Vec3 l, Vec3 r);


global f32  f32_epsilon =  1.1920929e-7f;
global f32  f32_pi      =  3.14159274f;
global f32  f32_tau     =  6.28318548f;

global f64  f64_epsilon =  2.2204460492503131e-16;
global f64  f64_pi      =  3.1415926535897931;
global f64  f64_tau     =  6.2831853071795862;

//function overloading????
function f32 f32_abs(f32 f);
function f64 f64_abs(f64 f);
//Macro or function???????
function b32 f32_compare(f32 x, f32 y);
function b32 f64_compare(f64 x, f64 y);

function f32 f32_sin(f32 x);
function f64 f64_sin(f64 x);

function f32 f32_sqrt(f32 x);
function f32 f64_sqrt(f32 x);

function f32 f32_cos(f32 x);
function f64 f64_cos(f64 x);

function f32 f32_tan(f32 x);
function f64 f64_tan(f64 x);


function f32 f32_inf(void);
function f64 f64_inf(void);
function f32 f32_neg_inf(void);
function f64 f64_neg_inf(void);

function f32 v2_dot(Vec2 l, Vec2 r);
function f32 v3_dot(Vec3 l, Vec3 r);
function Vec3 v3_normalize(Vec3 v);
function f32 v3_length(Vec3 v);

struct String8 {
    u8 *data;
    usize len;

    u8& operator[](usize index) {
        Assert(len > index);
        return data[index];
    }
};

struct String16 {
    u16 *data;
    usize len;

    u16& operator[](usize index) {
        Assert(len > index);
        return data[index];
    }
};

struct String32 {
    u32 *data;
    usize len;

    u32& operator[](usize index) {
        Assert(len > index);
        return data[index];
    }
};


function b32 char_is_lower(u8 c);
function b32 char_is_upper(u8 c);
function b32 char_is_alpha(u8 c);
function b32 char_is_space(u8 c);
function u8  char_to_upper(u8 c);
function u8  char_to_lower(u8 c);

struct Arena {
    u8 *buffer;
    usize cap;
    usize curr_offset;
    usize prev_offset; //may be useful
};

struct Console {
    void (*writef)(usize level, char *format, ...);
    void (*writef_error)(char *format, ...);
};

//global Console console = {};

#define array_len(x) (sizeof(x) / sizeof((x)[0]))

#define DEFAULT_ALIGNMENT (2*sizeof(void *))
function Arena arena_init(usize cap);
function void *arena_alloc(Arena *arena, usize asize, usize alignment = DEFAULT_ALIGNMENT);
function void arena_destroy(Arena *arena);


//intrinsics ????
extern "C" {
    void *memset(void *, int, size_t);
    void *memmove( void*, const void*, size_t);
}


#define memcopy(dst, src, size) memmove((dst), (src), (size))
#define memzero(s,z)       memset((s),0,(z))
#define memzero_struct(s)  memzero((s),sizeof(*(s)))

#define hv_swap(T, a, b) Statement(T __t = a; a = b; b = __t;)

#define make(T, arena_pointer, size) (T*)arena_alloc(arena_pointer, sizeof(T) * size);

#include "base.cpp"
