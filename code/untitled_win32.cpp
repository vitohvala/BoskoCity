/*
    TODO: audio
          linux
*/

//#include "base.cpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "base.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//#include "gl_renderer.h"
//#include "gl_renderer.cpp"

#include "game.h"
//#include "wglext.h"

#include <d3d11_4.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

#include <xaudio2.h>


#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


#define SAFE_RELEASE(ptr) Statement( if(ptr) ptr->Release(); )
#define HV_CREATE_WINDOW  (WM_USER + 0x0001)
#define HV_DESTROY_WINDOW (WM_USER + 0x0002)


/* ====================================================================

                                 GLOBAL

   ====================================================================*/
global b32 running;
global FILETIME shader_time;
global DWORD main_thread_id;
global b32 alloc_console = false;
//typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);
//    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
//

/* ====================================================================

                                STRUCTS

   ====================================================================*/


struct Win32Sound {
    WAVEFORMATEX wvf;
    XAUDIO2_BUFFER buffer;
    IXAudio2 *xaudio;
    IXAudio2MasteringVoice *mvoice;
    IXAudio2SourceVoice *srcvoice;
    f32 volume;
};

struct Win32GameCode{
    HMODULE dll;
    FILETIME last_write;
    UpdateP *update;
};

struct Win32GamePath{
    char dll[MAX_PATH];
    char tmp_dll[MAX_PATH];
};

struct Constants {
    Vec2 screensize;
    Vec2 atlassize;
};


struct DxContext{
    ID3D11Device *device;
    ID3D11DeviceContext *dcontext;
    ID3D11RenderTargetView *framebuffer_rtv;
    IDXGISwapChain4 *swapchain;
    D3D11_VIEWPORT viewport;
    ID3D11VertexShader *vertex_shader;
    ID3D11PixelShader *pixel_shader;
    ID3D11ShaderResourceView *sprite_SRV;
    ID3D11Buffer *sprite_buffer;
    ID3D11RasterizerState *rstate;
    ID3D11ShaderResourceView *atlas_SRV;
    ID3D11SamplerState *sampler;
    ID3D11Buffer *constant_buffer;
};

struct Win32Window {
    DWORD dwExStyle;
    LPCWSTR lpClassName;
    LPCWSTR lpWindowName;
    DWORD dwStyle;
    int X, Y;
    int nWidth, nHeight;
    HWND hWndParent;
    HMENU hMenu;
    HINSTANCE hInstance;
    LPVOID lpParam;
};

/* ====================================================================

                             FUNC DECLARATIONS

   ====================================================================*/
function LRESULT CALLBACK
win32_main_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* ====================================================================

                                FUNCTIONS

   ====================================================================*/

//void *platform_load_glfunc(char *func_name) {
//    void *proc = wglGetProcAddress(func_name);
//    hv_assert(proc != NULL);
//    return (void *)proc;
//}

void platform_swap_buffers(void *hdc) {
    SwapBuffers((HDC)hdc);
}

HWND
win32_create_window(u32 width, u32 height, char *name)
{
    log_info("Create Window\n");
    //remove HINSTANCE from func parametets?????
    HMODULE hinstance = (HINSTANCE)GetModuleHandleW(NULL);
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
        log_error("CreateWindowEx Failed\n");
        running = false;
    }

    log_info("Window created\n");
    return window_handle;
}


HWND
win32_create_bosko_window(u32 width, u32 height, char *name, HWND bosko_window)
{
    log_info("Create Window\n");
    //remove HINSTANCE from func parametets?????
    HMODULE hinstance = (HINSTANCE)GetModuleHandleW(NULL);
    WNDCLASSEXW wca = {};
    wca.cbSize = sizeof(wca);
    wca.lpfnWndProc = win32_main_proc;
    wca.hInstance = hinstance;
    wca.lpszClassName = L"UntitledClass";
    wca.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wca.hCursor = LoadCursor(0, IDC_ARROW);
    wca.hIcon = LoadIcon(NULL, IDI_WINLOGO);

    RECT rect = {0, 0, (LONG)width, (LONG)height};
    AdjustWindowRect(&rect, WS_VISIBLE | WS_OVERLAPPEDWINDOW, FALSE);
//
    //HWND window_handle = CreateWindowExW(0, wca.lpszClassName, (LPCWSTR)name,
    //                                 WS_OVERLAPPEDWINDOW,
    //                                 CW_USEDEFAULT, CW_USEDEFAULT,
    //                                 rect.right - rect.left, rect.bottom - rect.top,
    //                         0, 0, hinstance, 0);
//
    RegisterClassExW(&wca);
    Win32Window bosko = {};
    bosko.dwExStyle = 0;
    bosko.lpClassName = wca.lpszClassName;
    bosko.lpWindowName = (LPCWSTR)name;
    bosko.dwStyle = WS_OVERLAPPEDWINDOW;
    bosko.X = CW_USEDEFAULT;
    bosko.Y = CW_USEDEFAULT;
    bosko.nWidth = rect.right - rect.left;
    bosko.nHeight = rect.bottom - rect.top;
    bosko.hInstance = wca.hInstance;

    HWND window_handle = (HWND)SendMessageW(bosko_window, HV_CREATE_WINDOW, (WPARAM)&bosko, 0);


    if(!window_handle) {
        log_error("CreateWindowEx Failed\n");
        running = false;
    }

    log_info("Window created\n");
    return window_handle;
}


function void
win32_pump_msg(GameInput *old_input, GameInput *new_input, HWND tmp_window)
{
    ControllerInput *new_keyboard = &new_input->cinput[HV_Keyboard];
    ControllerInput *old_keyboard = &old_input->cinput[HV_Keyboard];
    memzero_struct(new_keyboard);

    for(int i = 0; i < (int)array_len(new_keyboard->buttons); i++) {
        new_keyboard->buttons[i].ended_down = old_keyboard->buttons[i].ended_down;
    }

    MSG msg = {};
    while(PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
        switch(msg.message) {
        case WM_KEYUP:
        case WM_KEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        {
            b32 was_down = ((msg.lParam & (1 << 30)) != 0);
            b32 is_down =  ((msg.lParam & (1 << 31)) == 0);
            WPARAM vk_code = msg.wParam;

            if(is_down != was_down) {
                switch(vk_code) {
                    case 'W':
                    case VK_UP: {
                        process_keyboard_message(&new_keyboard->move_up, is_down);
                    } break;
                    case 'S':
                    case VK_DOWN: {
                        process_keyboard_message(&new_keyboard->move_down, is_down);
                    } break;
                    case 'A':
                    case VK_LEFT: {
                        process_keyboard_message(&new_keyboard->move_left, is_down);
                    } break;
                    case 'D':
                    case VK_RIGHT: {
                        process_keyboard_message(&new_keyboard->move_right, is_down);
                    } break;
                    case VK_SPACE: {
                        process_keyboard_message(&new_keyboard->action_up, is_down);
                    } break;
                    case 'J': {
                        process_keyboard_message(&new_keyboard->action_right, is_down);
                    } break;
                    case 'K' : {
                        process_keyboard_message(&new_keyboard->action_down, is_down);
                    } break;
                    case 'H' : {
                        process_keyboard_message(&new_keyboard->action_left, is_down);
                    } break;
                    case 'R' : {
                        process_keyboard_message(&new_keyboard->start, is_down);
                    } break;
                }
            }
        } break;

        case WM_CLOSE: {
            SendMessageW(tmp_window, HV_DESTROY_WINDOW, msg.wParam, 0);
        } break;

        default :
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            break;
        }
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

#ifdef false
function HDC
win32_gl_prepare(HWND window_handle, int major_v, int minor_v)
{
    // /console.fatal("Test");
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

    {
        HINSTANCE hInstance = (HINSTANCE)GetModuleHandleW(NULL);
        HWND tmp_window_handle = win32_create_window(1280, 720, "nista");
        HDC tmp_dc = GetDC(tmp_window_handle);
        hv_assert(tmp_dc != NULL, "GetDC failed");

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
        hv_assert(pixel_format != 0);

        hv_assert(SetPixelFormat(tmp_dc, pixel_format, &pfd));

        HGLRC tmp_rc = wglCreateContext(tmp_dc);
        hv_assert(tmp_rc != NULL);

        hv_assert(wglMakeCurrent(tmp_dc, tmp_rc));

        wglChoosePixelFormatARB =
            (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_glfunc("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_glfunc("wglCreateContextAttribsARB");

        hv_assert(wglChoosePixelFormatARB || wglCreateContextAttribsARB);

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

    hv_assert(wglChoosePixelFormatARB != NULL);
    hv_assert(wglChoosePixelFormatARB(dc, pixel_attr, 0, 1,
                                   &pixel_format, &pixel_format_count));

    //log_infof("%d\n", pixel_format);

    PIXELFORMATDESCRIPTOR pfd = {};
    DescribePixelFormat(dc, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    hv_assert(SetPixelFormat(dc, pixel_format, &pfd));

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
    hv_assert(rc != 0);

    hv_assert(wglMakeCurrent(dc, rc));

    wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

    return dc;
}
#endif

function HRESULT
compile_shader(char *entrypoint, char *shader_model, ID3DBlob **blob_out)
{
    HRESULT res = S_OK;

     UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
    #ifndef NDEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    #else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    ID3DBlob *error_blob;
    //res = D3DCompile(hlsl, sizeof(hlsl), nil, nil, nil, entrypoint, shader_model, flags, 0, blob_out, nil);
    res = D3DCompileFromFile(L"assets/shader.hlsl", nil, nil, entrypoint, shader_model, flags,
                             0, blob_out, &error_blob);

    if (error_blob != nil) {
        log_warn((char*)error_blob->GetBufferPointer());
        error_blob->Release();
    }

    return res;
}

function void
d3d_free(DxContext *d)
{
    SAFE_RELEASE(d->device);
    SAFE_RELEASE(d->dcontext);
    SAFE_RELEASE(d->framebuffer_rtv);
    SAFE_RELEASE(d->swapchain);
    SAFE_RELEASE(d->vertex_shader);
    SAFE_RELEASE(d->pixel_shader);
    SAFE_RELEASE(d->sprite_SRV);
    SAFE_RELEASE(d->sprite_buffer);
    SAFE_RELEASE(d->rstate);
    SAFE_RELEASE(d->atlas_SRV);
    SAFE_RELEASE(d->sampler);
    SAFE_RELEASE(d->constant_buffer);
}

function DxContext
d3d_init(HWND handle) {
    DxContext d3d = {};
    {
        D3D_FEATURE_LEVEL feat_levels[] = { D3D_FEATURE_LEVEL_11_0 };

        ID3D11Device *base_device;
        ID3D11DeviceContext *base_device_ctx;

        uint creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        #ifdef DEBUG_BUILD
            creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        HRESULT res = D3D11CreateDevice(nil, D3D_DRIVER_TYPE_HARDWARE, nil, creation_flags, &feat_levels[0],
                        array_len(feat_levels), D3D11_SDK_VERSION, &base_device, nil, &base_device_ctx);
        hv_assert(SUCCEEDED(res), "CreateDevice failed");

        #ifdef DEBUG_BUILD
        {
            ID3D11InfoQueue* info;
            base_device->QueryInterface(IID_ID3D11InfoQueue, (void**)&info);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
            info->Release();
        }
        {
            IDXGIInfoQueue* dxgi_info;
            res = DXGIGetDebugInterface1(0, IID_IDXGIInfoQueue, (void**)&dxgi_info);
            hv_assert(SUCCEEDED(res));
            dxgi_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            dxgi_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
            dxgi_info->Release();
        }
        #endif


        res = base_device->QueryInterface(IID_ID3D11Device, (void**)&d3d.device);
        hv_assert(SUCCEEDED(res), "base_device->QueryInterface failed");

        res = base_device_ctx->QueryInterface(IID_ID3D11DeviceContext, (void**)&d3d.dcontext);
        hv_assert(SUCCEEDED(res), "base_device_ctx->QueryInterface failed");

        base_device->Release();
        base_device_ctx->Release();

        IDXGIDevice *dxgi_device;
        res = d3d.device->QueryInterface(IID_IDXGIDevice, (void**)&dxgi_device);
        hv_assert(SUCCEEDED(res), "d3d.device->QueryInterface failed");

        IDXGIAdapter *dxgi_adapter;
        res = dxgi_device->GetAdapter(&dxgi_adapter);
        hv_assert(SUCCEEDED(res), "dxgi getadapter  failed");

        DXGI_ADAPTER_DESC adapter_desc;
        res = dxgi_adapter->GetDesc(&adapter_desc);
        {
            char s_temp[128] = {};
            //TODO utf16 to 8
            int i = 0;
            for(i = 0; i < 128; i++) {
                if (adapter_desc.Description[i] > 31 && adapter_desc.Description[i] < 128) {
                    s_temp[i] = (char)adapter_desc.Description[i];
                }
            }
            s_temp[i - 1] = '\0';
            log_info("Graphics Device : %s\n", s_temp);
        }

        IDXGIFactory2 *dxgi_factory;
        res = dxgi_adapter->GetParent(IID_IDXGIFactory2, (void**)&dxgi_factory);
        hv_assert(SUCCEEDED(res), "dxgi adapter->GetParent  failed");

        dxgi_adapter->Release();
        dxgi_device->Release();

        DXGI_SWAP_CHAIN_DESC1 swd = {};
        swd.Width  = 0;
        swd.Height = 0;
        swd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swd.Stereo = false;
        swd.SampleDesc.Count = 1;
        swd.SampleDesc.Quality = 0;
        swd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swd.BufferCount = 2;
        swd.Scaling     = DXGI_SCALING_STRETCH;
        swd.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swd.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED;
        swd.Flags       = 0;

        //no msaa????

        IDXGISwapChain1 *swapchain;
        res = dxgi_factory->CreateSwapChainForHwnd(d3d.device, handle, &swd, nil, nil, &swapchain);
        hv_assert(SUCCEEDED(res), "CreateSwapChain Failed");

        dxgi_factory->MakeWindowAssociation(handle, DXGI_MWA_NO_ALT_ENTER);

        dxgi_factory->Release();

        swapchain->QueryInterface(IID_IDXGISwapChain4, (void **)&d3d.swapchain);
        hv_assert(SUCCEEDED(res), "Swapchain queryInterface failed");

        swapchain->Release();

        ID3D11Texture2D *framebuffer;
        res = d3d.swapchain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&framebuffer);
        hv_assert(SUCCEEDED(res), "GetBuffer failed");

        res = d3d.device->CreateRenderTargetView(framebuffer, nil, &d3d.framebuffer_rtv);
        hv_assert(SUCCEEDED(res), "CreateRenderTargetView failed");

        framebuffer->Release();

        d3d.dcontext->OMSetRenderTargets(1, &d3d.framebuffer_rtv, nil);


        {
            DXGI_SWAP_CHAIN_DESC1 swapchain_temp_desc;
            d3d.swapchain->GetDesc1(&swapchain_temp_desc);
            d3d.viewport = {};
            d3d.viewport.Width = (f32)swapchain_temp_desc.Width;
            d3d.viewport.Height = (f32)swapchain_temp_desc.Height;
            d3d.viewport.MaxDepth = 1;
            d3d.dcontext->RSSetViewports(1, &d3d.viewport);
            log_info("Swapchain window width %d\n", swapchain_temp_desc.Width);
            log_info("Swapchain window Height %d\n", swapchain_temp_desc.Height);
        }

        ID3DBlob *vs_blob;

        compile_shader("vs_main", "vs_5_0", &vs_blob);

        res = d3d.device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nil, &d3d.vertex_shader);
        hv_assert(SUCCEEDED(res), "CreateVertexShader failed");

        vs_blob->Release();

        ID3DBlob *ps_blob;

        compile_shader("ps_main", "ps_5_0", &ps_blob);

        res = d3d.device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nil, &d3d.pixel_shader);
        hv_assert(SUCCEEDED(res), "CreatePixelShader failed");

        ps_blob->Release();

        D3D11_RECT scissor_rect;
        scissor_rect.left = (LONG)d3d.viewport.TopLeftX;
        scissor_rect.top = (LONG)d3d.viewport.TopLeftY;
        scissor_rect.right = (LONG)(d3d.viewport.TopLeftX + d3d.viewport.Width);
        scissor_rect.bottom = (LONG)(d3d.viewport.TopLeftY + d3d.viewport.Height);

        d3d.dcontext->RSSetScissorRects(1, &scissor_rect);

        D3D11_RASTERIZER_DESC rdesc = {};
        rdesc.FillMode = D3D11_FILL_SOLID;
        rdesc.CullMode = D3D11_CULL_NONE;
        rdesc.FrontCounterClockwise = false;
        rdesc.DepthClipEnable = true;
        rdesc.MultisampleEnable = false;
        rdesc.AntialiasedLineEnable = false;
        rdesc.ScissorEnable = true;

        res = d3d.device->CreateRasterizerState(&rdesc, &d3d.rstate);
        hv_assert(SUCCEEDED(res), "CreateRasterizerState failed");

        /*
                might be useful */


        int twidth = 0, theight = 0, nr_channels = 0;
        u8 *image_data = (u8*)stbi_load("assets/atlas.png", &twidth, &theight, &nr_channels, 4);
        hv_assert(image_data != nil, "Image data is null");

        D3D11_TEXTURE2D_DESC texture_desc = {};
        texture_desc.Width      = u32(twidth);
        texture_desc.Height     = u32(theight);
        texture_desc.MipLevels  = 1;
        texture_desc.ArraySize  = 1;
        texture_desc.Format     = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture_desc.SampleDesc.Count = 1;
        texture_desc.Usage      = D3D11_USAGE_IMMUTABLE;
        texture_desc.BindFlags  = D3D11_BIND_SHADER_RESOURCE;


        D3D11_SUBRESOURCE_DATA texture_data = {};
        texture_data.pSysMem = &image_data[0];
        texture_data.SysMemPitch = (u32)twidth * 4;

        ID3D11Texture2D *texture = {};
        res = d3d.device->CreateTexture2D(&texture_desc, &texture_data, &texture);
        hv_assert(SUCCEEDED(res), "CreateTexture2d failed");

        d3d.device->CreateShaderResourceView(texture, nil, &d3d.atlas_SRV);

        D3D11_BUFFER_DESC sprite_buffer_desc = {};
        sprite_buffer_desc.ByteWidth           = MAX_SPRITES * sizeof(Sprite);
        sprite_buffer_desc.Usage               = D3D11_USAGE_DYNAMIC;
        sprite_buffer_desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
        sprite_buffer_desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        sprite_buffer_desc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        sprite_buffer_desc.StructureByteStride = sizeof(Sprite);

        d3d.device->CreateBuffer(&sprite_buffer_desc, nil, &d3d.sprite_buffer);

        D3D11_SHADER_RESOURCE_VIEW_DESC sprite_srv_desc = {};
        sprite_srv_desc.Format = DXGI_FORMAT_UNKNOWN;
        sprite_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        sprite_srv_desc.Buffer.NumElements = MAX_SPRITES;

        d3d.device->CreateShaderResourceView(d3d.sprite_buffer, &sprite_srv_desc, &d3d.sprite_SRV);

        D3D11_SAMPLER_DESC sampler_desc = {};
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

        res = d3d.device->CreateSamplerState(&sampler_desc, &d3d.sampler);
        hv_assert(SUCCEEDED(res), "Create sampler state failed");



        float constantData[4] = { d3d.viewport.Width, d3d.viewport.Height, (f32)twidth, (f32)theight };

        D3D11_BUFFER_DESC constant_buffer_desc = {};
        constant_buffer_desc.ByteWidth = sizeof(constantData) + 0xf & 0xfffffff0;
        constant_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE; // maybe change this???
        constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        D3D11_SUBRESOURCE_DATA constant_srd = {constantData };

        d3d.device->CreateBuffer(&constant_buffer_desc, &constant_srd, &d3d.constant_buffer);

        texture->Release();
        stbi_image_free(image_data);
    }

    return d3d;
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


//TODO: use string8????
function void
win32_console_write(char *text, usize level)
{
    //puts(text);
    HANDLE h =  GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD text_len = (DWORD)strlen(text);

    char *fstr[] =  { "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };
    //u8 levels[] = {6, 2, 1, 8};

    DWORD number_written = 0;
    //SetConsoleTextAttribute(h, levels[level]);
    WriteFile(h, fstr[level], (DWORD)strlen(fstr[level]), &number_written, 0);

    if (WriteFile(h, text, text_len, &number_written, 0) == 0) {
        LPVOID lpMsgBuf = format_err_msg(GetLastError());
        //puts((char *)lpMsgBuf);
    }

    OutputDebugStringA(text);
}

function void
win32_console_write2(char *text, usize level)
{
    //puts(text);
    HANDLE h =  GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD text_len = (DWORD)strlen(text);
    u8 levels[] = {6, 2, 1, 8};
    char *fstr[] =  { "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };

    SetConsoleTextAttribute(h, levels[level]);
    DWORD number_written = 0;
    WriteConsole(h, fstr[level], (DWORD)strlen(fstr[level]), &number_written, 0);
    SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    WriteConsole(h, text, text_len, &number_written, 0);

    //OutputDebugStringA(text);
}

function void
win32_write_error(char *text)
{
    #ifdef OS_GRAPHICAL
        DWORD dw = GetLastError();
        LPVOID lpMsgBuf = format_err_msg(dw);
        char errstr[1024];
        usize pstr_len = strlen(text);
        strncpy_s(errstr, sizeof(errstr), text, pstr_len);
        strncpy_s(errstr + pstr_len, sizeof(errstr) - pstr_len, (char *)lpMsgBuf,
                  strlen((char*)lpMsgBuf));
        int errmsg = MessageBoxA(0, errstr, "Error", MB_OK | MB_ICONERROR);

        LocalFree(lpMsgBuf);
        ExitProcess(dw);
    #else

        HANDLE h =  GetStdHandle(STD_ERROR_HANDLE);
        DWORD text_len = (DWORD)strlen(text);

        DWORD number_written = 0;
        WriteFile(h, "[ERROR]: ", 8, &number_written, 0);

        if (WriteFile(h, text, text_len, &number_written, 0) == 0) {
            LPVOID lpMsgBuf = format_err_msg(GetLastError());
            puts((char *)lpMsgBuf);
        }

        OutputDebugStringA(text);
    #endif
    running = 0;
}

function void
win32_writef_error(char *fmt, ...)
{
    char buffer[2048];
    va_list args;

    va_start(args, fmt);
    stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    win32_write_error(buffer);
}

function void
win32_console_writef(usize level, char *fmt, ...)
{
    char buffer[2048];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    win32_console_write(buffer, level);
    OutputDebugStringA(buffer);
}

function void
win32_console_writef2(usize level, char *fmt, ...)
{
    char buffer[2048];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    win32_console_write2(buffer, level);
    //OutputDebugStringA(buffer);
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

function inline b32
win32_hot_reload(Win32GameCode *game, Win32GamePath *p)
{
    FILETIME tmp_dll_filetime = win32_get_file_last_writetime(p->dll);

    if((CompareFileTime(&tmp_dll_filetime, &game->last_write)) != 0) {
        if(game->dll) {
            FreeLibrary(game->dll);
            game->dll = 0;
            game->update = NULL;
        }
        log_info("Game Code reloaded\n");
        win32_load_gamecode(p, game);
        return true;
    }
    return false;
}

function inline void
win32_hot_reload_shader(DxContext *d)
{
    FILETIME shader_tmp_ft = win32_get_file_last_writetime("assets/shader.hlsl");

    if((CompareFileTime(&shader_tmp_ft, &shader_time)) != 0) {

        log_info("shader reloaded\n");
        shader_time = win32_get_file_last_writetime("assets/shader.hlsl");
        HRESULT res = S_OK;
        ID3DBlob *vs_blob;

        res = compile_shader("vs_main", "vs_5_0", &vs_blob);
        if (FAILED(res) || !vs_blob) {
            log_warn("Vertex Shader Compilation failed\n");
            log_warn("Using old Vertex shader\n");
        } else {
            res = d->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nil, &d->vertex_shader);
            log_trace("Compiled Vertex Shader successfully\n");
            vs_blob->Release();
        }


        ID3DBlob *ps_blob;

        res = compile_shader("ps_main", "ps_5_0", &ps_blob);
        if (FAILED(res) || !ps_blob) {
            log_warn("Pixel Shader Compilation failed\n");
            log_warn("Using old Pixel shader\n");
        } else {
            res = d->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nil, &d->pixel_shader);
            //assert!!!
            log_trace("Compiled Pixel Shader successfully\n");

            ps_blob->Release();
        }

    }
}

function inline void
d3d_render(DxContext *d, SpriteBatch *sb, b32 vsync = true)
{
    D3D11_MAPPED_SUBRESOURCE spr_msr = {};

    d->dcontext->Map(d->sprite_buffer, 0,  D3D11_MAP_WRITE_DISCARD, nil, &spr_msr);
    {
        memcopy(spr_msr.pData, &sb->sprite[0], sb->count * sizeof(Sprite));
    }
    d->dcontext->Unmap(d->sprite_buffer, 0);

    d->dcontext->OMSetRenderTargets(1, &d->framebuffer_rtv, nullptr);

    d->dcontext->ClearRenderTargetView(d->framebuffer_rtv, (f32[]){0.0f, 0.0f, 0.0f, 1.0f});

    d->dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d->dcontext->RSSetState(d->rstate);

    d->dcontext->VSSetShader(d->vertex_shader, nil, 0);
    d->dcontext->VSSetShaderResources(0, 1, &d->sprite_SRV);
    d->dcontext->VSSetConstantBuffers(0, 1, &d->constant_buffer);
    d->dcontext->PSSetShader(d->pixel_shader, nil, 0);
    d->dcontext->PSSetShaderResources(1, 1, &d->atlas_SRV);
    d->dcontext->PSSetSamplers(0, 1, &d->sampler);

    d->dcontext->DrawInstanced(6, sb->count, 0, 0);

    d->swapchain->Present(vsync, {});
}

function Win32Sound
win32_xaudio_init(Arena *allocator)
{
    Win32Sound result = {};
    log_info("Initializing Xaudio2\n");

    result.volume = 1.0f;

    HRESULT res = S_OK;

    res = CoInitializeEx(nil, COINIT_MULTITHREADED);
    hv_assert(res == S_OK, "CoInitializeEx failed");

    WAVEFORMATEX wv = {};
    wv.wFormatTag = WAVE_FORMAT_PCM;
    wv.nChannels = 2;
    wv.nSamplesPerSec = 44100;
    wv.wBitsPerSample =  16;
    wv.nBlockAlign = wv.nChannels * 2;
    wv.nAvgBytesPerSec = wv.nSamplesPerSec * wv.nBlockAlign;

    res = XAudio2Create(&result.xaudio);
    hv_assert(res == S_OK, "Xaudio2create failed");

    res = result.xaudio->CreateMasteringVoice(&result.mvoice);
    hv_assert(res == S_OK, "CreateMasteringVocie failed");

    res = result.xaudio->CreateSourceVoice(&result.srcvoice, &wv);
    hv_assert(res == S_OK, "CreateSourceVoice Failed");
    res = result.srcvoice->SetVolume(result.volume);

    //memcopy();
    //check can set volume fail??
    //hv_assert(res == S_OK, "SetVolume Failed");

    return result;
}

function inline Console
win32_console_init()
{
    //char bbbb[256];

    Console logger = {};
    if(alloc_console) {
        AllocConsole();
        logger.writef = &win32_console_writef2;
    } else {
        logger.writef = &win32_console_writef;
    }

    logger.writef_error = &win32_writef_error;
    set_console(&logger);

    return logger;
}

static LRESULT CALLBACK
win32_tmp_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (uMsg) {
        case HV_CREATE_WINDOW: {
            Win32Window *win_window = (Win32Window *)wParam;
            result = (LRESULT)CreateWindowExW(win_window->dwExStyle, win_window->lpClassName,
                                              win_window->lpWindowName, win_window->dwStyle,
                                              win_window->X, win_window->Y,
                                              win_window->nWidth, win_window->nHeight,
                                              win_window->hWndParent, win_window->hMenu,
                                              win_window->hInstance, win_window->lpParam);
        } break;

        case HV_DESTROY_WINDOW:
        {
            DestroyWindow((HWND)wParam);
            running = false;
        } break;

        default:
        {
            result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
        } break;
    }

    return result;
}



function DWORD WINAPI
hv_main(LPVOID param)
{
    HWND bosko_tmp_window = (HWND)param;
    running = true;

    #if defined(DEBUG_BUILD)
        Console logger = win32_console_init();
    #else
        Console logger = {};
    #endif

    HWND window_handle = win32_create_bosko_window(1280, 720, "Bosko City", bosko_tmp_window);


    void *memory_buffer = VirtualAlloc((LPVOID)TB(2), GB(2),
                                       MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    Arena transient = arena_init(memory_buffer, MB(1000));
    Arena permanent = arena_init((void *)(transient.buffer + transient.cap),
                                 MB(1000));
    Arena temp_arena = arena_init((void *)(permanent.buffer + permanent.cap), MB(32));

    Win32GamePath gamepath = win32_load_gamepath();
    Win32GameCode game = {};
    win32_load_gamecode(&gamepath, &game);

    hv_assert(game.update != NULL);

    Memory mem = {};
    mem.transient = &transient;
    mem.permanent = &permanent;
    mem.temp = &temp_arena;
    mem.log = &logger;
    mem.is_init = false;

    DxContext d = d3d_init(window_handle);
    shader_time = win32_get_file_last_writetime("assets/shader.hlsl");

    win32_xaudio_init(&permanent);


    u64 pfreq = win32_get_perf_freq();
    u64 fcounter = win32_get_perf_counter();

    f32 dt = 0;

    GameInput input[2] = {};
    GameInput *old_input = &input[0];
    mem.input = &input[1];

    if(running && window_handle) {
        ShowWindow(window_handle, SW_SHOW);
        SetForegroundWindow(window_handle);
    }

    while(running) {
        if(win32_hot_reload(&game, &gamepath)) {
            mem.is_init = 2;
        }
        win32_hot_reload_shader(&d);

        win32_pump_msg(old_input, mem.input, bosko_tmp_window);

        if(game.update) game.update(&mem);

        d3d_render(&d, mem.sb);

        u64 ecounter = win32_get_perf_counter();
        dt = ((f32)ecounter - (f32)fcounter) / (f32)pfreq;
        fcounter = ecounter;
        mem.dt = dt;

        hv_swap(GameInput* , mem.input, old_input);

        arena_reset(&transient);
        arena_reset(&temp_arena);

    }

    if (window_handle) {
        DestroyWindow(window_handle);
    }

    d3d_free(&d);
    VirtualFree(memory_buffer, 0, MEM_RELEASE);
    ExitProcess(0);
}

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEXW wca = {};
    wca.cbSize = sizeof(wca);
    wca.lpfnWndProc = &win32_tmp_proc;
    wca.hInstance = GetModuleHandleW(NULL);
    wca.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    wca.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wca.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wca.lpszClassName = L"Untitled";
    RegisterClassExW(&wca);

    HWND service_window = CreateWindowExW(nil, wca.lpszClassName, L"tmp_BOSKO",
                                          WS_OVERLAPPEDWINDOW,
                                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                          0, 0, hInstance, 0);

    if(GetStdHandle(STD_OUTPUT_HANDLE) == NULL) {
        alloc_console = true;
    }

    CreateThread(0, 0, hv_main, service_window, 0, &main_thread_id);

    while(1) {
        MSG msg;
        GetMessageW(&msg, 0, 0, 0);
        TranslateMessage(&msg);
        if((msg.message == WM_CHAR) || (msg.message == WM_KEYDOWN) || (msg.message == WM_KEYUP) ||
           (msg.message == WM_QUIT) || (msg.message == WM_SIZE) || msg.message == WM_SETFOCUS ||
            msg.message == WM_KILLFOCUS || msg.message == WM_SYSCHAR || msg.message ==  WM_SYSKEYDOWN  ||
            msg.message == WM_SYSKEYUP )
        {
            PostThreadMessageW(main_thread_id, msg.message, msg.wParam, msg.lParam);
        } else {
            DispatchMessageW(&msg);
        }
    }
}

function LRESULT CALLBACK
win32_main_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;
    switch(uMsg) {
        case WM_CLOSE: {
            PostThreadMessageW(main_thread_id, uMsg, (WPARAM)hwnd, lParam);
            //running = false;
        } break;

        case WM_ERASEBKGND: return 1;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        //case WM_SIZE: {
        //    // Get the updated size.
        //    RECT r;
        //    GetClientRect(hwnd, &r);
        //    u32 width = r.right - r.left;
        //    u32 height = r.bottom - r.top;


        //} break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        case WM_MOUSEMOVE:
            // Mouse move
            //i32 x_position = GET_X_LPARAM(l_param);
            //i32 y_position = GET_Y_LPARAM(l_param);
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            PostThreadMessageW(main_thread_id, uMsg, (WPARAM)hwnd, lParam);
        } break;

        default : {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return result;
}