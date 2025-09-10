#version 430 core

uniform vec2 screensize;

struct IDK {
    vec2 pos;
    vec2 size;
};

struct Sprite {
    IDK sprite;
    IDK atlas;
    vec3 color;
};

layout(std430, binding = 0) buffer SpriteBuffer {
    Sprite sprites[];
};

out vec2 uv;
out vec4 vertex_color;

void main() {
    Sprite spr = sprites[gl_InstanceID];
    IDK sprite = spr.sprite;
    IDK atlas = spr.atlas;
    vec3 color = spr.color;

    vec2 vertices[6] = {
        sprite.pos,                                     // Top Left
        vec2(sprite.pos + vec2(0.0, sprite.size.y)),    // Bottom Left
        vec2(sprite.pos + vec2(sprite.size.x, 0.0)),    // Top Right
        vec2(sprite.pos + vec2(sprite.size.x, 0.0)),    // Top Right
        vec2(sprite.pos + vec2(0.0, sprite.size.y)),    // Bottom Left
        sprite.pos + sprite.size                        // Bottom Right
    };

    float left   = atlas.pos.x;
    float top    = atlas.pos.y;
    float right  = (atlas.pos.x + atlas.size.x);
    float bottom = (atlas.pos.y + atlas.size.y);

    vec2 uv_coords[6] = {
        vec2(left, top),
        vec2(left, bottom),
        vec2(right, top),
        vec2(right, top),
        vec2(left, bottom),
        vec2(right, bottom),
    };

    vec2 pos = 2.0f *  (vertices[gl_VertexID] / screensize) - 1.0;
    pos.y = -pos.y;

    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    uv = uv_coords[gl_VertexID];
    vertex_color = vec4(color, 1.0);
}
