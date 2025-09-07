#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <sys/mman.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

#include "base.h"

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
};

struct X11Window {
    Display *display;
    Window window;
};

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

global b32 running = false;
global GLContext gl_context;

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
    stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    console_write(buffer, level);
}

function void 
console_writef_error(char *fmt, ...) 
{
    char buffer[2048];
    va_list args;

    va_start(args, fmt);
    stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
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

function void 
gl_init(void) 
{
    log_info("Initializing OpenGL");
   
    gl_load_functions();
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glEnable(GL_DEBUG_OUTPUT); 

    uint vshader = glCreateShader(GL_VERTEX_SHADER);
    uint fshader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *vshader_src = "#version 430 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 0) out vec2 tex_coord;\n"
    //"out vec2 TexCoord;\n"
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
    "   gl_Position = vec4(vertices[gl_VertexID], 1.0f, 1.0f);\n"
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
            log_error("ERROR SHADER VERTEX COMPILATION_FAILED %s", infoLog);
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
            log_error("ERROR SHADER VERTEX COMPILATION_FAILED %s", infoLog);
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
            log_error("ERROR SHADER Program link FAILED %s", infoLog);
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
    u8 *data = (u8*)stbi_load("assets/atlas.png", &width, &height, &nr_channels, 0);

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

    log_info("VENDOR %s",  glGetString(GL_VENDOR));
    log_info("RENDERER %s",   glGetString(GL_RENDERER));
    log_info("VERSION %s",  glGetString(GL_VERSION));

    log_info("Finished Initializing OpenGL");
}

function void 
gl_render(X11Window *bosko)
{
    glClearColor(0.3, 0.7, 0.2, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glXSwapBuffers(bosko->display, bosko->window);

}

int main(){
    Console c = linux_console_init();
    running = true; 
   
    void *mem_buffer = mmap(0, GB(2), PROT_READ | PROT_WRITE, 
                            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    Arena transient = arena_init(mem_buffer, MB(1000));
    Arena permanent = arena_init((void*)(transient.buffer + transient.cap),
                                 MB(1000));
    Arena temp_arena = arena_init((void*)(permanent.buffer + permanent.cap), 
                                  MB(32));

    X11Window bosko = linux_create_window(1280, 720, "BoskoCity"); 

    Atom WM_DELETE_WINDOW = XInternAtom(bosko.display, "WM_DELETE_WINDOW", false);
    hv_assert(XSetWMProtocols(bosko.display, bosko.window, &WM_DELETE_WINDOW, 1) != 0, "could not register WM_DELETE_WINDOW property");    

    gl_init();
	glViewport(0, 0, 1280, 720);
    XEvent xevent;

    while(running) {
        //TODO: hot reload game and shaders
        if(XPending(bosko.display) > 0) {
            XNextEvent(bosko.display, &xevent);
            if(xevent.type == Expose) {
                XWindowAttributes xwa;
                XGetWindowAttributes(bosko.display, bosko.window, &xwa);
                glViewport(0, 0, xwa.width, xwa.height);    
            }
            if(xevent.type == ClientMessage) {
                if((Atom)xevent.xclient.data.l[0] == WM_DELETE_WINDOW) {
                    running = false;
                    
                }
            } 
            
        }
        gl_render(&bosko); 
        arena_reset(&transient);
    }
    
    arena_reset(&transient);
    arena_reset(&permanent);
    munmap(mem_buffer, GB(2));
    return 0;
}
