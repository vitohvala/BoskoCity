#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct GlContext {
    GLuint program_id;
    uint vao;
    uint transform_loc;
    void *hdc;
};

void platform_swap_buffers(void *hdc);

global GlContext gl_context;

function void APIENTRY
gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                  GLsizei length, const GLchar *message, const void *user)

{
    if(severity == GL_DEBUG_SEVERITY_LOW ||
       severity == GL_DEBUG_SEVERITY_MEDIUM ||
       severity == GL_DEBUG_SEVERITY_HIGH)
    {
        log_error("%s\n", (char*)message);
    } else {
        log_info("%s\n",  (char*)message);
    }
}

function void
gl_vport(u32 width, u32 height)
{
    glViewport(0, 0, width, height);
}

function b32
gl_init(Arena *arena, void *hdc)
{
    log_info("Init GL\n");

    gl_context.hdc = hdc;
    gl_load_functions();
    glDebugMessageCallback(&gl_debug_callback, 0);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);

    uint vshader = glCreateShader(GL_VERTEX_SHADER);
    uint fshader = glCreateShader(GL_FRAGMENT_SHADER);

    /*f32 vertices[] = {
         0.5f,  0.5f, 0.0f,  0.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom right
        -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top left
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f,// bottom left
    };*/

    const char *vshader_src = "#version 430 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 0) out vec2 tex_coord;\n"
    //"out vec2 TexCoord;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "   vec2 vertices[6] = {\n"
    "       vec2(-0.5, 0.5),\n"
    "       vec2(-0.5, -0.5),\n"
    "       vec2(0.5, 0.5),\n"
    "       vec2(0.5, 0.5),\n"
    "       vec2(-0.5, -0.5),\n"
    "       vec2( 0.5, -0.5),\n"
    "   };\n"
    "   float top = 0;\n"
    "   float left = 0;\n"
    "   float right = 512;\n"
    "   float bottom = 512;\n"
    "   vec2 tex_coords[6] = {\n"
    "       vec2(left, top),\n"
    "       vec2(left, bottom),\n"
    "       vec2(right, top),\n"
    "       vec2(right, top),\n"
    "       vec2(left, bottom),\n"
    "       vec2(right, bottom),\n"
    "   };\n"
    "   gl_Position = transform * vec4(vertices[gl_VertexID], 1.0f, 1.0f);\n"
    "   tex_coord = tex_coords[gl_VertexID];\n"
    "}\0";

    const char *fshader_src  = "#version 430 core\n"
    "out vec4 FragColor;\n"
    "layout (location = 0) in vec2 TexCoord;\n"
    "layout (location = 0) uniform sampler2D Texture;\n"
    "void main() { \n"
    "   vec4 texture_color = texelFetch(Texture, ivec2(TexCoord), 0);\n"
    "   if(texture_color.a == 0.0) discard;\n"
    "   FragColor = texture_color;\n"
    "}\0";

    glShaderSource(vshader, 1, &vshader_src, NULL);
    glCompileShader(vshader);

    {
        int  success;
        char infoLog[512];
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);

        if(!success) {
            glGetShaderInfoLog(vshader, 512, NULL, infoLog);
            log_error("ERROR SHADER VERTEX COMPILATION_FAILED %s\n", infoLog);
        }
    }

    glShaderSource(fshader, 1, &fshader_src, NULL);
    glCompileShader(fshader);

    {
        int  success;
        char infoLog[512];
        glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);

        if(!success) {
            glGetShaderInfoLog(fshader, 512, NULL, infoLog);
            log_error("ERROR SHADER VERTEX COMPILATION_FAILED %s\n", infoLog);
        }
    }

    gl_context.program_id = glCreateProgram();

    glAttachShader(gl_context.program_id, vshader);
    glAttachShader(gl_context.program_id, fshader);
    glLinkProgram(gl_context.program_id);

    {
        int  success;
        char infoLog[512];
        glGetProgramiv(gl_context.program_id, GL_LINK_STATUS, &success);

        if(!success) {
            glGetProgramInfoLog(gl_context.program_id, 512, NULL, infoLog);
            log_error("ERROR SHADER Program link FAILED %s\n", infoLog);
        }
    }

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    //uint vbo;
    glGenVertexArrays(1, &gl_context.vao);

    glUseProgram(gl_context.program_id);
    glBindVertexArray(gl_context.vao);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //stbi_set_flip_vertically_on_load(true);
    i32 width, height, nr_channels;
    u8 *data = (u8*)stbi_load("awesomeface.png", &width, &height, &nr_channels, 0);

    hv_assert(data != NULL, "stbi load failed");

    u32 texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    ///// set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);

    //glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    //glUniform1i(glGetUniformLocation(gl_context.program_id, "Texture"), 0);

    gl_context.transform_loc =
            glGetUniformLocation(gl_context.program_id, "transform");

    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(0x809D);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    arena_reset(arena);

    log_info("VENDOR %s\n",  glGetString(GL_VENDOR));
    log_info("RENDERER %s\n",   glGetString(GL_RENDERER));
    log_info("VERSION %s\n",  glGetString(GL_VERSION));

    log_info("GL Init complete\n");
    return true;
}

void
gl_render(Arena *transient, f32 t1)
{
    Mat4 *trans = (Mat4*)arena_alloc(transient, sizeof(Mat4));
    *trans = mat4(1.0f);

    f32 tx = f32_sin(t1);

    *trans = m4_translate(vec(0.0f, tx / 2, 0.0f));
    *trans *= vec(tx, 0.5f, 1.0f);
    *trans *= m4_rotate(*trans, vec(0.0f, 0.0f, 1.0f), t1);


    glClearColor(0.3f, 0.7f, 0.2f, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(gl_context.transform_loc, 1, GL_FALSE, (GLfloat*)trans->m);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    platform_swap_buffers(gl_context.hdc);

}