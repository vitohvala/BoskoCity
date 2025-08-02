

#include "base.h"
//#include "base.cpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "gl_renderer.h"
#include "gl_renderer.cpp"

#include "game.h"

#include "wglext.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

/* ====================================================================

                                 GLOBAL

   ====================================================================*/
global b32 running;

typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;


/* ====================================================================

                                STRUCTS

   ====================================================================*/

struct Win32GameCode{
    HMODULE dll;
    FILETIME last_write;
    UpdateP *update;
};

struct Win32GamePath{
    char dll[MAX_PATH];
    char tmp_dll[MAX_PATH];
};

/* ====================================================================

                             FUNC DECLARATIONS

   ====================================================================*/
//this should be the only function declaration
function LRESULT CALLBACK
win32_main_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


/* ====================================================================

                                FUNCTIONS

   ====================================================================*/

void *platform_load_glfunc(char *func_name) {
    void *proc = wglGetProcAddress(func_name);
    Assert(proc != NULL);
    return (void *)proc;
}


HWND
win32_create_window(HINSTANCE hInstance, u32 width, u32 height, char *name)
{
    //remove HINSTANCE from func parametets?????
    HMODULE hinstance = hInstance;
    WNDCLASSW wca = {};
    wca.lpfnWndProc = win32_main_proc;
    wca.hInstance = hinstance;
    wca.lpszClassName = L"UntitledClass";
    wca.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wca.hCursor = LoadCursor(0, IDC_ARROW);
    wca.hIcon = LoadIcon(NULL, IDI_WINLOGO);

    RECT rect = {0, 0, (LONG)width, (LONG)height};
    AdjustWindowRect(&rect, WS_VISIBLE | WS_OVERLAPPEDWINDOW, FALSE);

    RegisterClassW(&wca);
    HWND window_handle = CreateWindowExW(0, wca.lpszClassName, (LPCWSTR)name,
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     rect.right - rect.left, rect.bottom - rect.top,
                                     0, 0, hinstance, 0);

    if(!window_handle) {
        // log
        running = false;
    }

    return window_handle;

}

function void
win32_pump_msg(void)
{
    MSG msg = {};
    while(PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

function inline u64
win32_get_perf_counter(void)
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

function inline u64
win32_get_perf_freq(void)
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}

function HDC
win32_gl_prepare(HWND window_handle, int major_v, int minor_v)
{
    // /console.fatal("Test");
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

    {
        HINSTANCE hInstance = (HINSTANCE)GetModuleHandleW(NULL);
        HWND tmp_window_handle = win32_create_window(hInstance, 1280, 720, "nista");
        HDC tmp_dc = GetDC(tmp_window_handle);
        Assert(tmp_dc != NULL);

        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;

        int pixel_format = ChoosePixelFormat(tmp_dc, &pfd);
        Assert(pixel_format != 0);

        Assert(SetPixelFormat(tmp_dc, pixel_format, &pfd));

        HGLRC tmp_rc = wglCreateContext(tmp_dc);
        Assert(tmp_rc != NULL);

        Assert(wglMakeCurrent(tmp_dc, tmp_rc));

        wglChoosePixelFormatARB =
            (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_glfunc("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_glfunc("wglCreateContextAttribsARB");

        Assert(wglChoosePixelFormatARB || wglCreateContextAttribsARB);

        wglMakeCurrent(tmp_dc, 0);
        wglDeleteContext(tmp_rc);
        ReleaseDC(tmp_window_handle, tmp_dc);
        DestroyWindow(tmp_window_handle);
    }

    HDC dc = GetDC(window_handle);

    const int pixel_attr[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB,     32,
        WGL_ALPHA_BITS_ARB,     8,
        WGL_DEPTH_BITS_ARB,     24,
        0 // Terminate with 0, otherwise OpenGL will throw an Error!
    };

    uint pixel_format_count;
    int pixel_format = 0;

    Assert(wglChoosePixelFormatARB != NULL);
    Assert(wglChoosePixelFormatARB(dc, pixel_attr, 0, 1,
                                   &pixel_format, &pixel_format_count));

    //log_infof("%d\n", pixel_format);

    PIXELFORMATDESCRIPTOR pfd = {};
    DescribePixelFormat(dc, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    Assert(SetPixelFormat(dc, pixel_format, &pfd));

    #ifdef BUILD_DEBUG
        int opengl_debug = WGL_CONTEXT_DEBUG_BIT_ARB;
    #else
        int opengl_debug = 0;
    #endif

    const int context_attr[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major_v,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor_v,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_FLAGS_ARB, opengl_debug, 0
    };

    HGLRC rc = wglCreateContextAttribsARB(dc, 0, context_attr);
    Assert(rc != 0);

    Assert(wglMakeCurrent(dc, rc));

    wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

    return dc;
}

function LPVOID
format_err_msg(DWORD dw) {
    LPVOID lpMsgBuf;

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL) == 0) {
        MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
        ExitProcess(dw);
    }
    return lpMsgBuf;
}

function void
win32_console_write(const char *text)
{
    //puts(text);
    OutputDebugStringA(text);
}

function void
win32_console_writef(char *fmt, ...)
{
    char buffer[2048];
    va_list args;
    //TODO: my own vsnprintf
    va_start(args, fmt);
    stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    OutputDebugStringA(buffer);
    //arena_reset(arena);
}

function void
win32_log_fatal(char *str)
{
    //MessageBoxA(0, "Error", "Test", MB_OK);
    LPVOID lpMsgBuf = format_err_msg(GetLastError());
    char errstr[1024];
    usize pstr_len = strlen(str);
    strncpy_s(errstr, sizeof(errstr), str, pstr_len);
    strncpy_s(errstr + pstr_len, sizeof(errstr) - pstr_len, (char *)lpMsgBuf,
              strlen((char*)lpMsgBuf));
    int errmsg = MessageBoxA(0, errstr, "Error", MB_OK | MB_ICONERROR);

    LocalFree(lpMsgBuf);
    //ExitProcess(dw);

    running = 0;
}

DEBUG_FATAL(untitled_fatal) {
    win32_log_fatal(str);
}


function Win32GamePath
win32_load_gamepath(void) {
    char EXE_path[MAX_PATH];
    DWORD filename_size = GetModuleFileNameA(NULL, EXE_path, sizeof(EXE_path));
    char *OnePastLastSlash = EXE_path;
    for(char *scan = EXE_path + filename_size; ; --scan) {
        if(*scan == '\\') {
            OnePastLastSlash = scan + 1;
            break;
        }
    }
    usize EXEDirLength = OnePastLastSlash - EXE_path;
    Win32GamePath ret = {};
    char GameDLLName[] = "game.dll";
    //char GameDLLPath[MAX_PATH];
    strncpy_s(ret.dll, sizeof(ret.dll), EXE_path, EXEDirLength);
    strncpy_s(ret.dll + EXEDirLength,
              sizeof(ret.dll) - EXEDirLength, GameDLLName,
              sizeof(GameDLLName));

    char GameTempDLLName[] = "game_temp.dll";
    //char GameTempDLLPath[MAX_PATH];
    strncpy_s(ret.tmp_dll, sizeof(ret.tmp_dll), EXE_path, EXEDirLength);
    strncpy_s(ret.tmp_dll + EXEDirLength,
              sizeof(ret.tmp_dll) - EXEDirLength,
              GameTempDLLName, sizeof(GameTempDLLName));

    return ret;
}


function inline FILETIME
win32_get_file_last_writetime(char *FileName) {
    FILETIME Result = {};
    WIN32_FIND_DATA FindData;
    HANDLE Handle = FindFirstFileA(FileName, &FindData);
    if(Handle != INVALID_HANDLE_VALUE) {
        Result = FindData.ftLastWriteTime;
        FindClose(Handle);
    }

    return Result;
}

function void
win32_load_gamecode(Win32GamePath *p, Win32GameCode *game) {
    CopyFileA(p->dll, p->tmp_dll, FALSE);

    game->last_write = win32_get_file_last_writetime(p->dll);

    game->dll = LoadLibraryA(p->tmp_dll);

    if(game->dll) {
        game->update =
            (UpdateP *)GetProcAddress(game->dll, "game_update");
    }
}

function inline void
win32_hot_reload(Win32GameCode *game, Win32GamePath *p)
{
    FILETIME tmp_dll_filetime = win32_get_file_last_writetime(p->dll);

    if((CompareFileTime(&tmp_dll_filetime, &game->last_write)) != 0) {
        if(game->dll) {
            FreeLibrary(game->dll);
            game->dll = 0;
            game->update = NULL;
        }
        win32_load_gamecode(p, game);
    }
}

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine, int nCmdShow)
{
    running = true;
    HWND window_handle = win32_create_window(hInstance, 1280, 720, "Bosko City");

    void *memory_buffer = VirtualAlloc((LPVOID)TB(2), GB(2),
                                       MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    Arena transient = arena_init(memory_buffer, MB(1000));
    Arena permanent = arena_init((void *)(transient.buffer + transient.cap),
                                 MB(1000));
    Arena temp_arena = arena_init((void *)(permanent.buffer + permanent.cap), MB(32));

    //win32_log_fatal("test");
    Console c = {};
    c.fatal = &untitled_fatal;
    c.write = &win32_console_write;
    c.writef = &win32_console_writef;

    set_console(&c);

    Win32GamePath gamepath = win32_load_gamepath();
    Win32GameCode game = {};
    win32_load_gamecode(&gamepath, &game);

    Assert(game.update != NULL);

    Memory mem = {};
    mem.transient = transient;
    mem.permanent = permanent;
    mem.temp = temp_arena;

    mem.log = c;

    mem.is_init = false;

    HDC gl_dc = win32_gl_prepare(window_handle, 4, 5);
    gl_init(&temp_arena);
    gl_vport(1280, 720);

    //disable vsync
    //wglSwapIntervalEXT(0);

    log_infof("VENDOR %s\n",  glGetString(GL_VENDOR));
    log_infof("RENDERER %s\n",   glGetString(GL_RENDERER));
    log_infof("VERSION %s\n",  glGetString(GL_VERSION));

    ShowWindow(window_handle, SW_SHOW);

    u64 pfreq = win32_get_perf_freq();

    u64 fcounter = win32_get_perf_counter();
    u64 startc = fcounter;
    f32 t1 = 0.0f;

    f32 dt = 0;

    while(running) {
        win32_hot_reload(&game, &gamepath);

        win32_pump_msg();

        if(game.update) game.update(&mem);

        t1 = ((f32)win32_get_perf_counter() - (f32)startc) / (f32)pfreq;

        gl_render(&transient, t1);

        u64 ecounter = win32_get_perf_counter();
        dt = (f32)(ecounter - fcounter) / (f32)pfreq;

        fcounter = ecounter;

        f32 ms_per_frame = 1000 * dt;

        log_infof("ms = %.2f\n", ms_per_frame);

        SwapBuffers(gl_dc);
        arena_reset(&transient);
        arena_reset(&temp_arena);
    }

    if (window_handle) {
        DestroyWindow(window_handle);
    }

    VirtualFree(memory_buffer, 0, MEM_RELEASE);
    return 0;
}

function LRESULT CALLBACK
win32_main_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;
    switch(uMsg) {
        case WM_CLOSE: {
            running = false;
        } break;

        case WM_ERASEBKGND: return 1;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE: {
            // Get the updated size.
            RECT r;
            GetClientRect(hwnd, &r);
            u32 width = r.right - r.left;
            u32 height = r.bottom - r.top;

            gl_vport(width, height);
        } break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed/released
            //b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            // TODO: input processing

        } break;

        case WM_MOUSEMOVE: {
            // Mouse move
            //i32 x_position = GET_X_LPARAM(l_param);
            //i32 y_position = GET_Y_LPARAM(l_param);
            // TODO: input processing.
        } break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            //b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            // TODO: input processing.
        } break;

        default : {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return result;
}