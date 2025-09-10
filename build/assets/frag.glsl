#version 430 core
layout(location = 0) uniform sampler2D atlastexture;

in vec2 uv;
in vec4 vertex_color;

out vec4 FragColor;

void main() {
    //vec4 color = texture(atlastexture, uv);
    vec4 color = texelFetch(atlastexture, ivec2(uv), 0);
    if (color.a == 0.0) discard;
    FragColor = color * vertex_color;
}
