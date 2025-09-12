#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <sys/time.h>
#include <time.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

#include "base.h"

#include "game.h"

typedef void (APIENTRYP PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void (APIENTRYP PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
typedef void (APIENTRYP PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint *textures);
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);

#include "gl_renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct GLContext {
    GLuint program_id;
    uint vao;
    uint vshader;
    uint fshader;
    u32 screen_size_uloc;
    u32 sprite_id;
};

struct X11Window {
    Display *display;
    Window window;
};

union FullPath {
    struct {
        char main_so[PATH_MAX];
        char temp_so[PATH_MAX];
    };
    struct {
        char vshader_path[PATH_MAX];
        char fshader_path[PATH_MAX];
    };
};
struct LinuxGameCode {
    void *dll;
    struct stat last_write;
    UpdateP *update;
};

struct OpenGLShaderCode {
    struct stat last_vshader;
    struct stat last_fshader;
};

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
// TODO(vitalis):  remove some 
global b32 running = false;
global GLContext gl_context;
global Atom WM_DELETE_WINDOW;

void *platform_load_glfunc(char *func_name) {
    void *proc = (void *)glXGetProcAddress((const GLubyte *)func_name);
    hv_assert(proc != nil);
    return (void *)proc;
}

function void 
set_size_hints(Display *display, Window window, int minw, int minh, int maxw, int maxh) 
{
    XSizeHints hints;
    if(minw > 0 && minh > 0) hints.flags |= PMinSize;
    if(maxw > 0 && maxh > 0) hints.flags |= PMaxSize;
    
    hints.min_width = minw;
    hints.min_height = minh;
    hints.max_height = maxh;     
    hints.max_width = maxw;
    
    XSetWMNormalHints(display, window, &hints);
}

function void 
console_write(char *text, usize level) 
{
    char *fstr[] = {
        "\x1b[1m\x1B[38;5;226m[WARN]:\x1B[0m", 
        "\x1b[1m\x1B[38;5;48m[INFO]:\x1B[0m", 
        "\x1b[1m\x1B[38;5;97m[DEBUG]:\x1B[0m", 
        "\x1b[1m\x1B[38;5;102m[TRACE]:\x1B[0m"
    };
    
    fprintf(stdout, "%s %s\n", fstr[level], text);
}


function void 
console_write_error(char *text) 
{
    fprintf(stderr, "\x1b[1m\x1b[38;5;196m[ERROR]:\x1b[0m %s\n", text);
}

function void 
console_writef(usize level, char *fmt, ...) 
{
    char buffer[2048];
    va_list args;
    
    va_start(args, fmt);
    hv_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    console_write(buffer, level);
}

function void 
console_writef_error(char *fmt, ...) 
{
    char buffer[2048];
    va_list args;
    
    va_start(args, fmt);
    hv_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    console_write_error(buffer);
    running = false;
}

function inline Console
linux_console_init() 
{
    Console logger = {};
    logger.writef = &console_writef;
    logger.writef_error = &console_writef_error;
    set_console(&logger);
    return logger;    
}

function void APIENTRY 
gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, 
                  GLsizei length, const GLchar *message, const void *user) 
{
    if(severity == GL_DEBUG_SEVERITY_LOW || 
       severity == GL_DEBUG_SEVERITY_MEDIUM || 
       severity == GL_DEBUG_SEVERITY_HIGH) 
    {
        log_error("%s", (char*)message);
    } else {
        log_warn("%s", (char*)message);
    }
}

function u64 
linux_get_perf_counter(void)
{
    u64 result;
    struct timespec time_now = {};
    clock_gettime(CLOCK_MONOTONIC, &time_now);
    result =  time_now.tv_sec;
    result *= NS_PER_SECOND;
    result += time_now.tv_nsec;
    
    return result;
}

function X11Window
linux_create_window(int width, int height, char *name) 
{
    log_info("X11 initialization"); 
    
    Display *display = XOpenDisplay(nil);
    hv_assert(display != nil);
    
    Screen *screen = XDefaultScreenOfDisplay(display);
    int screen_id = XDefaultScreen(display);
    
    const int visual_attribs[] = {
        GLX_X_RENDERABLE, 1, 
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, 
        GLX_RENDER_TYPE, GLX_RGBA_BIT, 
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR, 
        GLX_RED_SIZE, 8, 
        GLX_GREEN_SIZE, 8, 
        GLX_BLUE_SIZE, 8, 
        GLX_ALPHA_SIZE, 8, 
        GLX_DEPTH_SIZE, 24, 
        GLX_STENCIL_SIZE, 8, 
        GLX_DOUBLEBUFFER, 1,
        None,
    };
    
    int majorx, minorx;
    glXQueryVersion(display, &majorx, &minorx);
    log_info("GLX majorx : %d GLX minorx : %d", majorx, minorx);
    if(majorx == 1 && minorx < 3) {
        log_warn("glx 1.3 or greater is required");
        running = false;
    }   
    
    int fb_count;
    GLXFBConfig *fbc = glXChooseFBConfig(display, screen_id, &visual_attribs[0], &fb_count);
    hv_assert(fbc != nil, "Failed to retrieve a framebuffer config");
    
    log_info("Found %d mathcing FB configs", fb_count);
    
    
    
    int best_fbc = -1, worst_fbc = -1, best_samp = -1, worst_samp = 999;
    
    for(int i = 0; i < fb_count; ++i) {
        XVisualInfo *vinfo = glXGetVisualFromFBConfig(display, fbc[i]);
        
        if(vinfo) {
            
            int samp_buf, samples;
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);
            
            if(best_fbc < 0 || (samp_buf && samples > best_samp)) {
                best_fbc = i;
                best_samp = samples;
            }
            if(worst_fbc < 0 || (!samp_buf && samples < worst_samp)) {
                worst_fbc = i;
                worst_samp = samples;
            }
        }
        
        XFree(vinfo);
        
    }
    
    GLXFBConfig bestfbc = fbc[best_fbc];
    
    XVisualInfo *visual = glXGetVisualFromFBConfig(display, bestfbc);
    hv_assert(visual != nil, "GetVisual failed");
    
    log_info("Choosen Visual ID : %d", visual->visualid);
    
    XFree(fbc);
    
    XSetWindowAttributes swa = {};
    swa.colormap = XCreateColormap(display, 
                                   XRootWindow(display, visual->screen), 
                                   visual->visual, AllocNone);
    swa.background_pixmap = None;
    swa.border_pixel = 0;
    swa.event_mask |= StructureNotifyMask | KeyPressMask | KeyReleaseMask | ExposureMask; 
    
    Window window = XCreateWindow(display, XRootWindow(display, visual->screen), 
                                  0, 0, 1280, 720, 0, visual->depth, InputOutput, visual->visual, 
                                  CWBorderPixel | CWColormap | CWEventMask, &swa);
    hv_assert(window != 0, "Failed to create window");
    
    //set_size_hints(display, window, 300, 300, 1280, 720);
    
    XFree(visual);
    
    XStoreName(display, window, "Bosko");
    XMapWindow(display, window);
    
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = 
    (glXCreateContextAttribsARBProc)glXGetProcAddressARB(
                                                         (const GLubyte *)"glXCreateContextAttribsARB");
    
    int context_attribs[] {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4, 
        GLX_CONTEXT_MINOR_VERSION_ARB, 3, 
        None,
    };
    
    GLXContext context = 0;
    const char *glx_ext = glXQueryExtensionsString(display, screen_id);
    if(!strstr(glx_ext, "GLX_ARB_create_context")) {
        log_warn("GLX_ARB create context not supported");
        context = glXCreateNewContext(display, bestfbc, GLX_RGBA_TYPE, 0, True);
    } else {
        context = glXCreateContextAttribsARB(display, bestfbc, 0, true, context_attribs);
    }
    XSync(display, false);
    
    if (!glXIsDirect (display, context)) {
        log_trace("Indirect GLX rendering context obtained");
    } else {
        log_trace("Direct GLX rendering context obtained");
    }
    
    glXMakeCurrent(display, window, context);
    
    
    log_info("Created X11 Window");
    X11Window result = {};
    result.window = window;
    result.display = display;
    return result;
}

// NOTE(vitalis): needs fullpath
function char *
linux_load_shader(Arena *temp, char *shader_path)
{
    int fd = open(shader_path, O_RDONLY);
    
    struct stat file_stat = {};
    fstat(fd, &file_stat);
    
    char *shader_src = make(char, temp, file_stat.st_size + 1);
    usize bytes_read = pread(fd, shader_src, file_stat.st_size, 0);
    shader_src[file_stat.st_size] = '\0';
    
    close(fd);
    
    return shader_src;
}

function b32
linux_copy_file(Arena *temp, char *filename1, char *filename2)
{
    int fd_f1 = open(filename1, O_RDONLY);
    int fd_f2 = open(filename2, O_WRONLY |  O_CREAT, 0755);
    
    if(fd_f1 == -1) {
        log_warn("failed to open %s", filename1);
        close(fd_f1);
        if(fd_f2 == -1) {
            log_warn("failed to open %s", filename2);
            close(fd_f2);
        }
        return false;
    }
    
    struct stat f1_stat = {};
    
    if(fstat(fd_f1, &f1_stat) == -1) {
        log_warn("failed to stat %s", filename1, strerror(errno));
        close(fd_f2);
        close(fd_f1);
        return false;
    }
    
    char *tmp_buffer = make(char, temp, f1_stat.st_size + 1);
    Assert(tmp_buffer != nil);
    usize bytes_read = pread(fd_f1, tmp_buffer, f1_stat.st_size, 0);
    tmp_buffer[f1_stat.st_size] = '\0';
    
    log_info("Copying %s into %s", filename1, filename2);
    usize bytes_written = pwrite(fd_f2, tmp_buffer, f1_stat.st_size, 0);
    
    close(fd_f1);
    close(fd_f2);
    
    arena_reset(temp);
    if(!bytes_written || ! bytes_read) {
        log_warn("copyng failed bytes read : %d bytes written : %d : %s", bytes_read, bytes_written, strerror(errno));
    }
    return (bytes_written > 0 && bytes_read > 0);
}

function LinuxGameCode
linux_load_gamecode(Arena *temp, FullPath *game_dll_paths, String8 *exe_dir) 
{
    LinuxGameCode result = {};
    
    int try_copy = 1;
    while(!linux_copy_file(temp, game_dll_paths->main_so, game_dll_paths->temp_so)){
        try_copy++;
        log_info("Copying attempt %d failed, trying again", try_copy);
        usleep(10000);
    }
    
    stat(game_dll_paths->main_so, &result.last_write);
    
    result.dll = dlopen(game_dll_paths->temp_so, RTLD_NOW);
    
    if(!result.dll) {
        log_error("Can't load %s", game_dll_paths->temp_so);
    } else {
        dlerror();
        result.update = (UpdateP *)dlsym(result.dll, "game_update");
        if(!result.update) {
            log_warn("%s", dlerror());
        } else 
            log_info("Gamecode Loaded");
    }
    arena_reset(temp);
    return result;
}

function uint 
gl_compile_shader(Arena *arena, char *path, GLenum shader_type)
{
    uint shader = glCreateShader(shader_type);
    
    int success = 0;
    char info_log[512] = {};
    
    char *shader_src = linux_load_shader(arena, path);
    
    glShaderSource(shader, 1, &shader_src, nil);
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    arena_reset(arena);
    
    if(!success) {
        glGetShaderInfoLog(shader, 512, nil, info_log);
        log_warn("Shader compilation failed %s", info_log);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

function void 
gl_init(Arena *temp, FullPath *shader_paths, String8 *exe_dir) 
{
    log_info("Initializing OpenGL");
    
    gl_load_functions();
    glDebugMessageCallback(&gl_debug_callback, 0);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glEnable(GL_DEBUG_OUTPUT); 
    
    uint vshader = gl_compile_shader(temp, shader_paths->vshader_path, GL_VERTEX_SHADER);
    uint fshader = gl_compile_shader(temp, shader_paths->fshader_path, GL_FRAGMENT_SHADER);
    
    hv_assert(vshader != 0 && fshader != 0, "failed compiling shaders");
    
    gl_context.program_id = glCreateProgram();
    
    glAttachShader(gl_context.program_id, vshader);
    glAttachShader(gl_context.program_id, fshader);
    
    gl_context.vshader = vshader;
    gl_context.fshader = fshader;
    
    glLinkProgram(gl_context.program_id);
    
    int success = 0;
    char info_log[512] = {};
    
    glGetProgramiv(gl_context.program_id, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(gl_context.program_id, 512, NULL, info_log);
        log_error("Shader program link failed %s", info_log);
    }
    
    //glDeleteShader(vshader);
    //glDetachShader(gl_context.program_id, vshader);
    //glDeleteShader(fshader);
    //glDetachShader(gl_context.program_id, fshader);
    
    //uint vbo;
    glGenVertexArrays(1, &gl_context.vao);
    
    glUseProgram(gl_context.program_id);
    glBindVertexArray(gl_context.vao);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    //stbi_set_flip_vertically_on_load(true);
    
    usize atlas_path_len = hv_strlen("assets/atlas.png");
    
    char atlas_path[PATH_MAX];
    
    memcopy(atlas_path, exe_dir->data, exe_dir->len);
    memcopy(atlas_path + exe_dir->len, "assets/atlas.png", atlas_path_len);
    atlas_path[exe_dir->len + atlas_path_len] = '\0'; 
    
    i32 width, height, nr_channels;
    
    u8 *data = (u8*)stbi_load(atlas_path, &width, &height, &nr_channels, 0);
    
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
    
    
    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(0x809D);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    
    gl_context.sprite_id = 0;
    glGenBuffers(1, &gl_context.sprite_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl_context.sprite_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sprite) * MAX_SPRITES, nil, GL_DYNAMIC_DRAW);
    
    gl_context.screen_size_uloc = glGetUniformLocation(gl_context.program_id, "screensize");
    
    Vec2 scsize = {1280.0f, 720.0f};
    glUniform2fv(gl_context.screen_size_uloc, 1, (GLfloat*)&scsize);
    
    log_info("VENDOR %s",  glGetString(GL_VENDOR));
    log_info("RENDERER %s",   glGetString(GL_RENDERER));
    log_info("VERSION %s",  glGetString(GL_VERSION));
    
    log_info("Finished Initializing OpenGL");
}

function FullPath
linux_find_fullpath(String8 *exe_dir, char *filename1, char *filename2)
{
    usize filename1_len = hv_strlen(filename1);
    usize filename2_len = hv_strlen(filename2);
    
    FullPath result = {};
    memcopy(result.main_so, exe_dir->data, exe_dir->len);
    memcopy(result.main_so + exe_dir->len, filename1, filename1_len);
    result.main_so[exe_dir->len + filename1_len] = '\0';
    
    memcopy(result.temp_so, exe_dir->data, exe_dir->len);
    memcopy(result.temp_so + exe_dir->len, filename2, filename2_len);
    result.temp_so[exe_dir->len + filename2_len] = '\0';
    
    return result;
}

function b32 
linux_hot_reload(FullPath *gamepath, LinuxGameCode *game) 
{
    struct stat tmp_stat;
    stat(gamepath->main_so, &tmp_stat);
    
    if(tmp_stat.st_mtim.tv_sec != game->last_write.st_mtim.tv_sec) {
        if(game->dll) {
            dlclose(game->dll);
            //game = 0;
            game->dll = 0;
            game->update = 0;
            //game->last_write = tmp_stat;
        }
        
        return true;
    }
    return false;
}

function b32
gl_link_program(void)
{
    glLinkProgram(gl_context.program_id);
    
    int success = 0;
    char info_log[512] = {};
    
    glGetProgramiv(gl_context.program_id, GL_LINK_STATUS, &success);
    
    if(!success) {
        glGetProgramInfoLog(gl_context.program_id, 512, nil, info_log);
        log_warn("Program linking failed %s, rolling back to old shaders");
        return false;
    }
    return true;
}

function void 
linux_reload_shader(Arena *arena, char *path, uint *cntx_shader,GLenum shader_type)
{
    uint shader = gl_compile_shader(arena, path, shader_type);
    
    if(shader != 0) {
        glDetachShader(gl_context.program_id, *cntx_shader);
        glAttachShader(gl_context.program_id, shader);
        if(shader_type == GL_VERTEX_SHADER) {
            log_info("Vertex Shader reloaded");
        } else if(shader_type == GL_FRAGMENT_SHADER) {
            log_info("Fragment Shader reloaded");
        }
        
        if(gl_link_program()){
            *cntx_shader = shader;
        } else {
            glAttachShader(gl_context.program_id, *cntx_shader);
            glDetachShader(gl_context.program_id, shader);
            glLinkProgram(gl_context.program_id);
        }
    }
}

function void 
linux_hot_reload_shaders(Arena *arena, FullPath *fpath, String8 *exe_dir, OpenGLShaderCode *scode)
{
    // TODO(vitalis): CLEANUP!!!!!!!!!!
    struct stat tmp_shader_stat = {};
    stat(fpath->vshader_path, &tmp_shader_stat);
    
    
    if(tmp_shader_stat.st_mtim.tv_sec != scode->last_vshader.st_mtim.tv_sec) {
        scode->last_vshader = tmp_shader_stat;
        
        linux_reload_shader(arena, fpath->vshader_path, &gl_context.vshader, GL_VERTEX_SHADER);
    }
    
    stat(fpath->fshader_path, &tmp_shader_stat);
    
    if(tmp_shader_stat.st_mtim.tv_sec != scode->last_fshader.st_mtim.tv_sec) {
        scode->last_fshader = tmp_shader_stat;
        
        linux_reload_shader(arena, fpath->fshader_path, &gl_context.fshader,GL_FRAGMENT_SHADER);
    }
}

function void 
gl_render(X11Window *bosko, SpriteBatch *sb)
{
    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    
    glUseProgram(gl_context.program_id);
    glBindVertexArray(gl_context.vao);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_context.sprite_id);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Sprite) * sb->count, sb->sprite);
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (i32)sb->count);
    glXSwapBuffers(bosko->display, bosko->window);
    
}

function String8 
linux_find_exe_dir(Arena *arena)
{
    String8 result = {};
    //result.data = make(u8, arena, PATH_MAX);
    char buffer[PATH_MAX] = {};
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    //result.len = len;
    if (len > -1) {
        for(int i = len; i > 0 ; --i) {
            if(buffer[i] == '/') {
                buffer[i + 1] = '\0';
                break;
            }
        }
    }
    
    usize buffer_len = hv_strlen(buffer);
    result.len = buffer_len;
    result.data = make(u8, arena, buffer_len);
    
    memcopy(result.data, buffer, buffer_len);
    
    return result;
}

function void 
linux_pump_msg(X11Window *w, GameInput *new_input, GameInput* old_input, Vec2 *screensize)
{
    ControllerInput *new_keyboard = &new_input->cinput[HV_Keyboard];
    ControllerInput *old_keyboard = &old_input->cinput[HV_Keyboard];
    
    memzero_struct(new_keyboard);
    
    for(int i = 0; i < (int)array_len(new_keyboard->buttons); i++) {
        new_keyboard->buttons[i].ended_down = old_keyboard->buttons[i].ended_down;
    }
    
    XEvent xevent;
    if(XPending(w->display) > 0) {
        XNextEvent(w->display, &xevent);
        switch(xevent.type) {
            case Expose : {
                XWindowAttributes xwa;
                XGetWindowAttributes(w->display, w->window, &xwa);
                
                Vec2 scsize = {(f32)xwa.width, (f32)xwa.height};
                glUniform2fv(gl_context.screen_size_uloc, 1, (GLfloat*)&scsize);
                *screensize = scsize; 
                glViewport(0, 0, xwa.width, xwa.height);    
            } break;
            
            case ClientMessage :  {
                if((Atom)xevent.xclient.data.l[0] == WM_DELETE_WINDOW) {
                    running = false;
                }
            }  break;
            
            case KeyPress:
            case KeyRelease: {
                KeySym key = XLookupKeysym(&xevent.xkey, 0);
                b32 is_down = (xevent.type == KeyPress);
                b32 was_down = (xevent.type == KeyRelease);
                
                b32 alt_pressed = (xevent.xkey.state & (Mod1Mask | Mod5Mask));
                
                if(is_down != was_down) {
                    switch(key) {
                        case XK_a:
                        case XK_Left: {
                            process_keyboard_message(&new_keyboard->move_left, is_down);
                        } break;
                        case XK_s:
                        case XK_Down: {
                            process_keyboard_message(&new_keyboard->move_down, is_down);
                        } break;
                        case XK_d:
                        case XK_Right: {
                            process_keyboard_message(&new_keyboard->move_right, is_down);
                        } break;
                        case XK_w: 
                        case XK_Up: {
                            process_keyboard_message(&new_keyboard->move_up, is_down);
                        } break;
                        case XK_space: {
                            process_keyboard_message(&new_keyboard->action_up, is_down);
                        } break;
                        case XK_j: {
                            process_keyboard_message(&new_keyboard->action_right, is_down);
                        } break;
                        case XK_k : {
                            process_keyboard_message(&new_keyboard->action_down, is_down);
                        } break;
                        case XK_h: {
                            process_keyboard_message(&new_keyboard->action_left, is_down);
                        } break;
                        case XK_r: {
                            process_keyboard_message(&new_keyboard->start, is_down);
                        } break;
                    }
                }
            } break;
        }
    }
}

int main(void){
    Console c = linux_console_init();
    running = true;
    
    void *mem_buffer = mmap(0, GB(2), PROT_READ | PROT_WRITE, 
                            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    
    Arena transient = arena_init(mem_buffer, MB(1000));
    Arena permanent = arena_init((void*)(transient.buffer + transient.cap),
                                 MB(1000));
    Arena temp_arena = arena_init((void*)(permanent.buffer + permanent.cap), 
                                  MB(32));
    
    String8 exe_dir = linux_find_exe_dir(&permanent);
    FullPath gamecode_path = linux_find_fullpath(&exe_dir, "game.so", "game_temp.so");
    FullPath shaders_path = linux_find_fullpath(&exe_dir, "assets/vertex.glsl", "assets/frag.glsl");
    OpenGLShaderCode shader_code = {};
    
    stat(shaders_path.vshader_path, &shader_code.last_vshader);
    stat(shaders_path.fshader_path, &shader_code.last_fshader);
    
    X11Window bosko = linux_create_window(1280, 720, "BoskoCity"); 
    
    WM_DELETE_WINDOW = XInternAtom(bosko.display, "WM_DELETE_WINDOW", false);
    hv_assert(XSetWMProtocols(bosko.display, bosko.window, &WM_DELETE_WINDOW, 1) != 0, "could not register WM_DELETE_WINDOW property");    
    
    gl_init(&transient, &shaders_path, &exe_dir);
    glViewport(0, 0, 1280, 720);
    
    LinuxGameCode game = linux_load_gamecode(&temp_arena, &gamecode_path, &exe_dir);
    
    Memory game_mem;
    game_mem.temp = &temp_arena;
    game_mem.transient = &transient;
    game_mem.permanent = &permanent;
    game_mem.log = &c;
    game_mem.dt = 0;
    game_mem.is_init = false;
    
    GameInput input[2] = {};
    GameInput *old_input = &input[0];
    game_mem.input = &input[1];
    game_mem.screen_size = {1280, 720};
    
    u64 start_frame = linux_get_perf_counter();
    //f32 dt = {};
    
    while(running) {
        if(linux_hot_reload(&gamecode_path, &game)) {
            game = linux_load_gamecode(&temp_arena, &gamecode_path, &exe_dir);
            game_mem.is_init = 2;
            //game.update = 0;
        }
        linux_hot_reload_shaders(&temp_arena, &shaders_path, &exe_dir, &shader_code);
        
        linux_pump_msg(&bosko, game_mem.input, old_input, &game_mem.screen_size);
        
        if(game.update) game.update(&game_mem);
        else {
            log_warn("game.update is null attempting to load again");
            game = linux_load_gamecode(&temp_arena, &gamecode_path, &exe_dir);
            game_mem.is_init = 2;
        }
        gl_render(&bosko, game_mem.sb); 
        
        u64 end_frame = linux_get_perf_counter();
        game_mem.dt = ((f32)end_frame - (f32)start_frame) / (f32)NS_PER_SECOND;
        //log_trace("dt = %f", game_mem.dt);
        start_frame = end_frame;
        
        hv_swap(GameInput*, game_mem.input, old_input);
        
        arena_reset(&transient);
        arena_reset(&temp_arena);
    }
    
    arena_reset(&transient);
    arena_reset(&permanent);
    munmap(mem_buffer, GB(2));
    return 0;
}
