//#include "base.cpp"

function inline  f32
f32_abs(f32 f)
{
    return f < 0 ? -f : f;
}

function inline  f64
f64_abs(f64 f)
{
    return f < 0 ? -f : f;
}

function inline  b32
f32_compare(f32 x, f32 y)
{
    return (f32_abs(x - y) <= f32_epsilon * MAX(1.0f, MAX(x, y)));
}

function inline  b32
f64_compare(f64 x, f64 y)
{
    return (f64_abs(x - y) <= f64_epsilon * MAX(1.0, MAX(x, y)));
}

function inline  f32
f32_radians(f32 degree)
{
    return degree * (f32_pi / 180.0f);
}

//TODO: write my own math lib
#include <math.h>

//TODO: should this be inline???
function inline f32
f32_sin(f32 x)
{
    return sinf(x);
}

function inline f64
f64_sin(f64 x)
{
    return sin(x);
}

function inline f32
f32_cos(f32 x)
{
    return cosf(x);
}

function inline f64
f64_cos(f64 x)
{
    return cos(x);
}

function inline f32
f32_tan(f32 x)
{
    return tanf(x);
}

function inline f64
f64_tan(f64 x)
{
    return tan(x);
}

function inline f32
f32_inf(void)
{
    //TODO:
    return 0.0f;
}
function inline f64
f64_inf(void)
{
    return 0.0;
}

function inline f32
f32_neg_inf(void)
{
    return 0.0f;
}

function inline f64
f64_neg_inf(void)
{
    return 0.0;
}

function inline Vec2
operator+(Vec2 l, Vec2 r)
{
    Vec2 ret = { l.x + r.x, l.y + r.y };
    return ret;
}

function inline Vec2
operator+=(Vec2 &l, Vec2 r)
{
    l = l + r;
    return l;
}

function inline Vec2
operator-(Vec2 l)
{
    Vec2 result = {-l.x, -l.y};
    return result;
}

function inline Vec2
operator-(Vec2 l, Vec2 r)
{
    Vec2 ret = { l.x - r.x, l.y - r.y };
    return ret;
}

function inline Vec2
operator-=(Vec2 &l, Vec2 r)
{
    l = l - r;
    return l;
}

function inline Vec2
operator*(Vec2 l, Vec2 r)
{
    Vec2 ret = {l.x * r.x, l.y * r.y};
    return ret;
}

function inline Vec2
operator*(Vec2 l, f32 r)
{
    Vec2 ret = {l.x * r, l.y * r};
    return ret;
}

function inline Vec2
operator*=(Vec2 &l, Vec2 r)
{
    l = l * r;
    return l;
}

function inline b32
operator==(Vec2 l, Vec2 r)
{
    //TODO: maybe use compare function inline
    return (l.x == r.x && l.y == r.y);
}

function inline b32
operator!=(Vec2 l, Vec2 r)
{
    return !(l == r);
}


function inline Vec3
operator+(Vec3 l, Vec3 r)
{
    Vec3 result = { l.x + r.x, l.y + r.y, l.z + r.z };
    return result;
}

function inline Vec3
operator+=(Vec3 &l, Vec3 r)
{
    l = l + r;
    return l;
}

function inline Vec3
operator-(Vec3 l, Vec3 r)
{
    Vec3 result = { l.x - r.x, l.y - r.y, l.z - r.z };
    return result;
}

function inline Vec3
operator-(Vec3 l)
{
    Vec3 result = { -l.x, -l.y, -l.z };
    return result;
}

function inline Vec3
operator-=(Vec3 &l, Vec3 r)
{
    l = l - r;
    return l;
}

function inline Vec3
operator*(Vec3 l, Vec3 r)
{
    Vec3 result = { l.x * r.x, l.y * r.y, l.z * r.z };
    return result;
}

function inline Vec3
operator*=(Vec3 &l, Vec3 r)
{
    l = l * r;
    return l;
}

function inline Vec3
operator*(Vec3 l, f32 r)
{
    Vec3 result = { l.x * r, l.y * r, l.z * r };
    return result;
}

function inline Vec3
operator*=(Vec3 &l, f32 r)
{
    l = l * r;
    return l;
}

function inline b32
operator==(Vec3 l, Vec3 r)
{
    //TODO use compare function inline
    return (l.x == r.x && l.y == r.y && l.z == r.z);
}
function inline b32
operator!=(Vec3 l, Vec3 r)
{
    return !(l == r);
}

function inline Vec2
vec(f32 x, f32 y)
{
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

function inline Vec3
vec(f32 x, f32 y, f32 z)
{
    Vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

function inline Vec4
vec(f32 x, f32 y, f32 z, f32 w)
{
    Vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

function inline Vec2
vec2(f32 x)
{
    Vec2 result;
    result.x = x;
    result.y = x;
    return result;
}

function inline Vec3
vec3(f32 x)
{
    Vec3 result;
    result.x = x;
    result.y = x;
    result.z = x;
    return result;
}

function inline Vec4
vec4(f32 x)
{
    Vec4 result;
    result.x = x;
    result.y = x;
    result.z = x;
    result.w = x;
    return result;
}

function inline f32
f32_sqrt(f32 x)
{
    return sqrtf(x);
}

function inline f32
v3_length(Vec3 v)
{
    return f32_sqrt(v3_dot(v, v));
}

function inline Vec3
v3_normalize(Vec3 v)
{
    Vec3 v1 = {};
    //Assert(v3_length(v) > 0);

    v1 = v;

    f32 l = v3_length(v1);

    if(l != 0.0f && l != 1.0f) {
        f32 ilen = 1.0f / f32_sqrt(l);
        v1 *= ilen;
    }
    //Vec3 v1 = v * (1.0f / v3_length(v));
    return v1;
}

function inline Vec4
operator+(Vec4 l, Vec4 r)
{
    Vec4 result = {l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w};
    return result;
}

function inline Vec4
operator+=(Vec4 &l, Vec4 r)
{
    l = l + r;
    return l;
}

function inline Vec4
operator-(Vec4 l, Vec4 r)
{
    Vec4 result = {l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w};
    return result;
}

function inline Vec4
operator-=(Vec4 &l, Vec4 r)
{
    l = l - r;
    return l;
}

function inline Vec4
operator-(Vec4 l)
{
    Vec4 result = {-l.x, -l.y, -l.z, -l.w};
    return result;
}
function inline Vec4
operator*(Vec4 l, Vec4 r)
{
    Vec4 result = {l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w};
    return result;
}

function inline Vec4
operator*(Vec4 l, f32 r)
{
    Vec4 result = {l.x * r, l.y * r, l.z * r, l.w * r};
    return result;
}

function inline Vec4
operator*=(Vec4 &l, Vec4 r)
{
    l = l * r;
    return l;
}

function inline Mat4
operator+(Mat4 x, Mat4 y)
{
    Mat4 result = {};
    result.m0 = x.m0 + y.m0;
    result.m1 = x.m1 + y.m1;
    result.m2 = x.m2 + y.m2;
    result.m3 = x.m3 + y.m3;

    return result;
}

function inline Mat4
operator-(Mat4 l, Mat4 r)
{
    Mat4 result = {};
    for (usize i = 0; i < 4; i++) {
        result.m[i] = l.m[i] - r.m[i];
    }
    return result;
}

function inline Mat4
operator*(Mat4 l, Vec3 scale)
{
    Mat4 result = l;

    result.m0.x *= scale.x;
    //result.m0.y *= scale.x;
    //result.m0.z *= scale.x;

    //result.m1.x *= scale.y;
    result.m1.y *= scale.y;
    //result.m1.z *= scale.y;

    //result.m2.x *= scale.z;
    //result.m2.y *= scale.z;
    result.m2.z *= scale.z;


    result.m3.w = 1.0f;
    return result;
}

function inline Mat4
operator*(Mat4 l, Mat4 r)
{
    Mat4 result;

    for (int i = 0; i < 4; i++) {
        result.m[i].x = l.m[i].x * r.m[0].x + l.m[i].y * r.m[1].x + l.m[i].z * r.m[2].x + l.m[i].w * r.m[3].x;
        result.m[i].y = l.m[i].x * r.m[0].y + l.m[i].y * r.m[1].y + l.m[i].z * r.m[2].y + l.m[i].w * r.m[3].y;
        result.m[i].z = l.m[i].x * r.m[0].z + l.m[i].y * r.m[1].z + l.m[i].z * r.m[2].z + l.m[i].w * r.m[3].z;
        result.m[i].w = l.m[i].x * r.m[0].w + l.m[i].y * r.m[1].w + l.m[i].z * r.m[2].w + l.m[i].w * r.m[3].w;
    }

    return result;
}
/*
function inline Vec4
operator=(Vec4 r)
{
    Vec4 result = {};
    result;
}

function inline Mat4
operator=(Mat4 l, Mat4 r)
{
    Mat4 result = {};
    for(int i = 0; i < 4; i++) {
        l.m[i] = r.m[i];
    }
    return l;
}*/


function inline Mat4
operator*=(Mat4 &l, Vec3 scale)
{
    l = l * scale;
    return l;
}


function inline Mat4
operator*=(Mat4 &l, Mat4 r)
{
    l = l * r;
    return l;
}

function inline Mat4
m4_scale(Mat4 l, Vec3 scale)
{
    return l * scale;
}

function inline Mat4
mat4(f32 x)
{
    Mat4 m = {};
    m.m0.x = 1.0f;
    m.m1.y = 1.0f;
    m.m2.z = 1.0f;
    m.m3.w = 1.0f;

    return m;
}

function inline Mat4
m4_rotate(Mat4 mat, Vec3 v, f32 angle_radians)
{
    f32 c = f32_cos(angle_radians);
    f32 s = f32_sin(angle_radians);

    Mat4 rot = mat;

    Vec3 axis = v3_normalize(v);

    f32 x = axis.x, y = axis.y, z = axis.z;

    f32 t = 1.0f - c;

    rot.m0.x = x*x*t + c;
    rot.m0.y = y*x*t + z*s;    // swapped
    rot.m0.z = z*x*t - y*s;    // swapped

    rot.m1.x = x*y*t - z*s;    // swapped
    rot.m1.y = y*y*t + c;
    rot.m1.z = z*y*t + x*s;    // swapped

    rot.m2.x = x*z*t + y*s;    // swapped
    rot.m2.y = y*z*t - x*s;    // swapped
    rot.m2.z = z*z*t + c;
    rot.m3.w = 1.0f;

	return (rot);
}

function inline Mat4
m4_translate(Vec3 v)
{
    Mat4 result = mat4(1.0f);
    Assert(result.m0.x == 1 && result.m1.y == 1);
    Assert(result.m2.z == 1 && result.m3.w == 1);
    result.m3.x = v.x;
    result.m3.y = v.y;
    result.m3.z = v.z;
    return result;
}

function inline f32
v2_dot(Vec2 l, Vec2 r)
{
    return (f32)(l.x * r.x) + (l.y * r.y);
}

function inline f32
v3_dot(Vec3 l, Vec3 r)
{
    return (f32)(l.x * r.x) + (l.y * r.y) + (l.z * r.y);
}

function inline b32
char_is_lower(u8 c)
{
    return (c >= 'a' && c <= 'z');
}

function inline b32
char_is_upper(u8 c)
{
    return (c >= 'A' && c <= 'Z');
}

function inline b32
char_is_alpha(u8 c)
{
    return (char_is_lower(c) || char_is_upper(c));
}

function inline b32
char_is_space(u8 c)
{
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r');
}

function inline u8
char_to_upper(u8 c)
{
    if(char_is_lower(c)) {
        return (c ^ 32);
    }
    return c;
}

function inline u8
char_to_lower(u8 c)
{
    if(char_is_upper(c)) {
        return c | 32;
    }
    return c;
}

function inline b32
is_power_of_two(uptr x)
{
	return (x & (x-1)) == 0;
}

function inline uptr
align_forward(uptr ptr, usize align)
{
	uptr p, a, modulo;

	Assert(is_power_of_two(align));

	p = ptr;
	a = (uptr)align;
	// Same as (p % a) but faster as 'a' is a power of two
	modulo = p & (a-1);

	if (modulo != 0) {
		// If 'p' address is not aligned, push the address to the
		// next value which is aligned
		p += a - modulo;
	}
	return p;
}

function  void
*arena_alloc(Arena *a, usize size, usize align)
{
	// Align 'curr_offset' forward to the specified alignment
	uptr curr_ptr = (uptr)a->buffer + (uptr)a->curr_offset;
	uptr offset = align_forward(curr_ptr, align);
	offset -= (uptr)a->buffer; // Change to relative offset

	// Check to see if the backing memory has space left
	if (offset+size <= a->cap) {
		void *ptr = &a->buffer[offset];
		a->prev_offset = offset;
		a->curr_offset = offset+size;

		// Zero new memory by default
		memset(ptr, 0, size);
		return ptr;
	}
	// Return NULL if the arena is out of memory (or handle differently)
	return NULL;
}

function Arena
arena_init(void *buffer, usize cap)
{
    Arena a = {};
    a.buffer = (u8*)buffer;
    a.cap = cap;
    a.curr_offset = 0;
    a.prev_offset = 0;

    return a;
}

function void
arena_reset(Arena *arena)
{
    arena->curr_offset = arena->prev_offset = 0;
}

global Console console;

#define log_warn(...)  Statement( if(console.writef) { console.writef(0, __VA_ARGS__); } )
#define log_info(...)  Statement( if(console.writef) { console.writef(1, __VA_ARGS__); } )
#define log_debug(...) Statement( if(console.writef) { console.writef(2, __VA_ARGS__); } )
#define log_trace(...) Statement( if(console.writef) { console.writef(3, __VA_ARGS__); } )
#define log_error(...) Statement( if(console.writef_error) { console.writef_error(__VA_ARGS__);  } )
#define fatal(...)     Statement( if(console.writef_error) { console.writef_error(__VA_ARGS__);  } )

function void
set_console(Console *c)
{
    console = *c;
}

//(TODO): popravi ovo malo
function inline
void hv_assert(b32 assertion, const char *fmt = nil, ...) {
    if (!assertion) {
        if(fmt) {
            char buffer[2048];
            va_list args;

            va_start(args, fmt);
            hv_vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);

            log_error(buffer);
        } else {
            log_error("Assertion:  ");
        }
    }
    //    Assert(assertion);
}

function usize
hv_strlen(const char *text)
{
    usize len = 0;
    for(usize i = 0; text[i] != '\0'; i++) {
        len++;
    }
    return len;
}

function String8
str8(Arena *a, char *text)
{
    String8 result;

    result.len = hv_strlen(text);
    result.data = (u8*)arena_alloc(a, result.len);
    result.data = (u8*)text;
    return result;
}

function String8
tmp_format_str8(Arena *allocator, const char *fmt, ...)
{
    String8 buffer;
    buffer.data = (u8*)arena_alloc(allocator, 2048);

    va_list args;

    va_start(args, fmt);
    buffer.len = hv_vsnprintf((char*)buffer.data, 2047, fmt, args);
    va_end(args);

    return buffer;
}
