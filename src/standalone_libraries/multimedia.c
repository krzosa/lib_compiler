#include "multimedia.h"

#ifndef MU_StaticFunc
    #if defined(__GNUC__) || defined(__clang__)
        #define MU_StaticFunc __attribute__((unused)) static
    #else
        #define MU_StaticFunc static
    #endif
#endif

#ifndef MU_PRIVATE_VAR
    #define MU_PRIVATE_VAR MU_StaticFunc
#endif

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <shellapi.h> // for handling dropped files

    #define INITGUID
    #define CINTERFACE
    #define COBJMACROS
    #define CONST_VTABLE
    #include <objbase.h>
    #include <audioclient.h>
    #include <audiopolicy.h>
    #include <mmdeviceapi.h>

    //
    // Automatically linking with the libraries
    //
    #pragma comment(lib, "gdi32.lib")
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "shell32.lib") // For handling dropping files into the app
#endif
MU_StaticFunc void *MU_PushSize(MU_Arena *ar, size_t size);
MU_StaticFunc MU_UTF32Result MU_UTF16ToUTF32(uint16_t *c, int max_advance);
MU_StaticFunc MU_UTF8Result MU_UTF32ToUTF8(uint32_t codepoint);
MU_StaticFunc int64_t MU_CreateCharFromWidechar(char *buffer, int64_t buffer_size, wchar_t *in, int64_t inlen);
MU_StaticFunc void MU_WIN32_UpdateFocusedWindowBasedOnHandle(MU_Context *mu, HWND handle);
MU_StaticFunc void MU_WIN32_UpdateFocusedWindow(MU_Context *mu);
MU_StaticFunc void MU_Win32_GetWindowSize(HWND window, int *x, int *y);
MU_StaticFunc void MU_WIN32_GetWindowPos(HWND window, int *x, int *y);
MU_StaticFunc MU_Int2 MU_WIN32_GetMousePosition(HWND window);
MU_StaticFunc MU_Int2 MU_WIN32_GetMousePositionInverted(HWND window, int y);
MU_StaticFunc void MU_WIN32_CreateCanvas(MU_Window *window);
MU_StaticFunc void MU_WIN32_DestroyCanvas(MU_Window *window);
MU_StaticFunc void MU_WIN32_DrawCanvas(MU_Window *window);
MU_StaticFunc void MU__MemoryCopy(void *dst, const void *src, size_t size);
MU_StaticFunc void MU_PushDroppedFile(MU_Context *mu, MU_Window *window, char *str, int size);
MU_StaticFunc void MU_UpdateWindowState(MU_Window *window);
MU_StaticFunc int MU__AreStringsEqual(const char *src, const char *dst, size_t dstlen);
MU_StaticFunc void *MU_Win32_GLGetWindowProcAddressForGlad(const char *proc);
MU_StaticFunc void MU_WIN32_GetWGLFunctions(MU_Context *mu);
MU_StaticFunc void MU_WIN32_TryToInitGLContextForWindow(MU_Context *mu, MU_Win32_Window *w32_window);
MU_StaticFunc void MU_WIN32_DeinitSound(MU_Context *mu);
MU_StaticFunc void MU_WIN32_LoadCOM(MU_Context *mu);
MU_StaticFunc DWORD MU_WIN32_SoundThread(void *parameter);
MU_StaticFunc void MU_WIN32_InitWasapi(MU_Context *mu);

#ifndef MU_GL_ENABLE_MULTISAMPLING
    #define MU_GL_ENABLE_MULTISAMPLING 1
#endif

#ifndef MU_GL_BUILD_DEBUG
    #define MU_GL_BUILD_DEBUG 1
#endif

#ifndef MU_ASSERT_CODE
    #define MU_ASSERT_CODE(x) x
#endif

#ifndef MU_ASSERT
    #include <assert.h>
    #define MU_ASSERT(x) assert(x)
#endif

/* Quake uses this to sleep when user is not interacting with app
    void SleepUntilInput (int time)
    {
        MsgWaitForMultipleObjects(1, &tevent, FALSE, time, QS_ALLINPUT);
    }

    if ((cl.paused && (!ActiveApp && !DDActive)) || Minimized || block_drawing)
    {
        SleepUntilInput (PAUSE_SLEEP);
        scr_skipupdate = 1;     // no point in bothering to draw
    }
    else if (!ActiveApp && !DDActive)
    {
        SleepUntilInput (NOT_FOCUS_SLEEP);
    }
    */
// @! Add native handle to MU_Context for Directx 11 initialize
// @! Add option to manually blit, some manual blit param and manual blit function
// @! Add ram info? https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex
// @! Maybe make the library friendly to people who dont use debuggers
// @! Set window title
// @! Good defaults for multiple windows ??

struct MU_UTF32Result {
    uint32_t out_str;
    int advance;
    int error;
};

struct MU_UTF8Result {
    char out_str[4];
    int len;
    int error;
};

#define MU_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define MU_STACK_ADD_MOD(stack_base, new_stack_base, next) \
    do {                                                   \
        (new_stack_base)->next = (stack_base);             \
        (stack_base) = (new_stack_base);                   \
    } while (0)
#define MU_STACK_ADD(stack_base, new_stack_base) \
    MU_STACK_ADD_MOD(stack_base, new_stack_base, next)

MU_API void MU_Quit(MU_Context *mu) {
    mu->quit = true;
}

MU_API void MU_DefaultSoundCallback(MU_Context *mu, uint16_t *buffer, uint32_t samples_to_fill) {
}

MU_INLINE void MU__WriteChar8(MU_Window *window, char *c, int len) {
    if (window->user_text8_count + len < MU_ARRAY_SIZE(window->user_text8)) {
        for (int i = 0; i < len; i += 1) {
            window->user_text8[window->user_text8_count++] = c[i];
        }
    }
}

MU_INLINE void MU__WriteChar32(MU_Window *window, uint32_t c) {
    if (window->user_text32_count + 1 < MU_ARRAY_SIZE(window->user_text32)) {
        window->user_text32[window->user_text32_count++] = c;
    }
}

MU_INLINE void MU__KeyDown(MU_Window *window, MU_Key key) {
    if (window->key[key].down == false)
        window->key[key].press = true;
    window->key[key].down = true;
    window->key[key].raw_press = true;
}

MU_INLINE void MU__ZeroMemory(void *p, size_t size) {
    uint8_t *p8 = (uint8_t *)p;
    for (size_t i = 0; i < size; i += 1) {
        p8[i] = 0;
    }
}

MU_INLINE size_t MU__GetAlignOffset(size_t size, size_t align) {
    size_t mask = align - 1;
    size_t val = size & mask;
    if (val) {
        val = align - val;
    }
    return val;
}

MU_INLINE void MU__KeyUP(MU_Window *window, MU_Key key) {
    if (window->key[key].down == true)
        window->key[key].unpress = true;
    window->key[key].down = false;
}

MU_INLINE bool MU_DoesSizeFit(MU_Arena *ar, size_t size) {
    const size_t alignment = 16;
    size_t align = MU__GetAlignOffset((uintptr_t)ar->memory + ar->len, alignment);
    size_t cursor = ar->len + align;
    bool result = cursor + size <= ar->cap;
    return result;
}

#define MU_PUSH_STRUCT(mu, T) (T *)MU_PushSize(mu, sizeof(T))
MU_StaticFunc void *MU_PushSize(MU_Arena *ar, size_t size) {
    const size_t alignment = 16;

    ar->len += MU__GetAlignOffset((uintptr_t)ar->memory + ar->len, alignment);
    if (ar->len + size > ar->cap) {
        MU_ASSERT(!"MU_Context has not enough memory for what you are trying to do!");
    }

    void *result = (void *)(ar->memory + ar->len);
    ar->len += size;
    MU_ASSERT(ar->len < ar->cap);
    MU__ZeroMemory(result, size);
    return result;
}

MU_StaticFunc MU_UTF32Result MU_UTF16ToUTF32(uint16_t *c, int max_advance) {
    MU_UTF32Result result;
    MU__ZeroMemory(&result, sizeof(result));
    if (max_advance >= 1) {
        result.advance = 1;
        result.out_str = c[0];
        if (c[0] >= 0xD800 && c[0] <= 0xDBFF && c[1] >= 0xDC00 && c[1] <= 0xDFFF) {
            if (max_advance >= 2) {
                result.out_str = 0x10000;
                result.out_str += (uint32_t)(c[0] & 0x03FF) << 10u | (c[1] & 0x03FF);
                result.advance = 2;
            }
            else
                result.error = 2;
        }
    }
    else {
        result.error = 1;
    }
    return result;
}

MU_StaticFunc MU_UTF8Result MU_UTF32ToUTF8(uint32_t codepoint) {
    MU_UTF8Result result;
    MU__ZeroMemory(&result, sizeof(result));
    if (codepoint <= 0x7F) {
        result.len = 1;
        result.out_str[0] = (char)codepoint;
    }
    else if (codepoint <= 0x7FF) {
        result.len = 2;
        result.out_str[0] = 0xc0 | (0x1f & (codepoint >> 6));
        result.out_str[1] = 0x80 | (0x3f & codepoint);
    }
    else if (codepoint <= 0xFFFF) { // 16 bit word
        result.len = 3;
        result.out_str[0] = 0xe0 | (0xf & (codepoint >> 12)); // 4 bits
        result.out_str[1] = 0x80 | (0x3f & (codepoint >> 6)); // 6 bits
        result.out_str[2] = 0x80 | (0x3f & codepoint);        // 6 bits
    }
    else if (codepoint <= 0x10FFFF) { // 21 bit word
        result.len = 4;
        result.out_str[0] = 0xf0 | (0x7 & (codepoint >> 18));  // 3 bits
        result.out_str[1] = 0x80 | (0x3f & (codepoint >> 12)); // 6 bits
        result.out_str[2] = 0x80 | (0x3f & (codepoint >> 6));  // 6 bits
        result.out_str[3] = 0x80 | (0x3f & codepoint);         // 6 bits
    }
    else {
        result.error = true;
    }

    return result;
}

// @warning: this function is a little different from usual, returns -1 on decode errors
MU_StaticFunc int64_t MU_CreateCharFromWidechar(char *buffer, int64_t buffer_size, wchar_t *in, int64_t inlen) {
    int64_t outlen = 0;
    for (int64_t i = 0; i < inlen && in[i] != 0;) {
        MU_UTF32Result decode = MU_UTF16ToUTF32((uint16_t *)(in + i), (int)(inlen - i));
        if (!decode.error) {
            i += decode.advance;
            MU_UTF8Result encode = MU_UTF32ToUTF8(decode.out_str);
            if (!encode.error) {
                for (int64_t j = 0; j < encode.len; j++) {
                    if (outlen >= buffer_size) {
                        outlen = -1;
                        goto failed_to_decode;
                    }
                    buffer[outlen++] = encode.out_str[j];
                }
            }
            else {
                outlen = -1;
                goto failed_to_decode;
            }
        }
        else {
            outlen = -1;
            goto failed_to_decode;
        }
    }

    buffer[outlen] = 0;
failed_to_decode:;
    return outlen;
}

#ifdef _WIN32
    #define MU_DEFAULT_MEMORY_SIZE (1024 * 4)

// Typedefines for the COM functions which are going to be loaded currently only required for sound
typedef HRESULT CoCreateInstanceFunction(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
typedef HRESULT CoInitializeExFunction(LPVOID pvReserved, DWORD dwCoInit);

struct MU_Win32 {
    WNDCLASSW wc;
    bool good_scheduling;
    MU_glGetProcAddress *wgl_get_proc_address;
    HMODULE opengl32;

    HCURSOR cursor_hand;
    HCURSOR cursor_arrow;

    // Sound
    IMMDevice *device;
    IAudioClient *audio_client;
    IMMDeviceEnumerator *device_enum;
    IAudioRenderClient *audio_render_client;

    uint32_t buffer_frame_count;
    // IAudioCaptureClient *audio_capture_client;

    // Pointers to the functions from the dll
    CoCreateInstanceFunction *CoCreateInstanceFunctionPointer;
    CoInitializeExFunction *CoInitializeExFunctionPointer;
};

struct MU_Win32_Window {
    HDC handle_dc;
    HDC canvas_dc;
    HBITMAP canvas_dib;

    // for fullscreen
    WINDOWPLACEMENT prev_placement;
    DWORD style;
};

MU_API double MU_GetTime(void) {
    static int64_t counts_per_second;
    if (counts_per_second == 0) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        counts_per_second = freq.QuadPart;
    }

    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    double result = (double)time.QuadPart / (double)counts_per_second;
    return result;
}

MU_StaticFunc void MU_WIN32_UpdateFocusedWindowBasedOnHandle(MU_Context *mu, HWND handle) {
    if (mu->all_windows == 0) return;
    for (MU_Window *it = mu->all_windows; it; it = it->next) {
        if (it->handle == handle) {
            mu->window = it;
            return;
        }
    }
}

MU_StaticFunc void MU_WIN32_UpdateFocusedWindow(MU_Context *mu) {
    HWND handle = GetFocus();
    if (handle) {
        MU_WIN32_UpdateFocusedWindowBasedOnHandle(mu, handle);
    }
}

MU_StaticFunc void MU_Win32_GetWindowSize(HWND window, int *x, int *y) {
    RECT window_rect;
    GetClientRect(window, &window_rect);
    *x = window_rect.right - window_rect.left;
    *y = window_rect.bottom - window_rect.top;
}

MU_StaticFunc void MU_WIN32_GetWindowPos(HWND window, int *x, int *y) {
    POINT point = {0, 0};
    ClientToScreen(window, &point);
    *x = point.x;
    *y = point.y;
}

MU_StaticFunc MU_Int2 MU_WIN32_GetMousePosition(HWND window) {
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(window, &p);
    MU_Int2 result = {p.x, p.y};
    return result;
}

MU_StaticFunc MU_Int2 MU_WIN32_GetMousePositionInverted(HWND window, int y) {
    MU_Int2 result = MU_WIN32_GetMousePosition(window);
    result.y = y - result.y;
    return result;
}

MU_StaticFunc void MU_WIN32_CreateCanvas(MU_Window *window) {
    MU_Win32_Window *w32_window = (MU_Win32_Window *)window->platform;

    MU_ASSERT(window->canvas == 0);

    BITMAPINFO bminfo;
    {
        MU__ZeroMemory(&bminfo, sizeof(bminfo));
        bminfo.bmiHeader.biSize = sizeof(bminfo.bmiHeader);
        bminfo.bmiHeader.biWidth = (LONG)window->size.x;
        bminfo.bmiHeader.biHeight = (LONG)window->size.y;
        bminfo.bmiHeader.biPlanes = 1;
        bminfo.bmiHeader.biBitCount = 32;
        bminfo.bmiHeader.biCompression = BI_RGB; // AA RR GG BB
        bminfo.bmiHeader.biXPelsPerMeter = 1;
        bminfo.bmiHeader.biYPelsPerMeter = 1;
    }
    w32_window->canvas_dib = CreateDIBSection(w32_window->handle_dc, &bminfo, DIB_RGB_COLORS, (void **)&window->canvas, 0, 0);
    w32_window->canvas_dc = CreateCompatibleDC(w32_window->handle_dc);
}

MU_StaticFunc void MU_WIN32_DestroyCanvas(MU_Window *window) {
    MU_Win32_Window *w32_window = (MU_Win32_Window *)window->platform;
    if (window->canvas) {
        DeleteDC(w32_window->canvas_dc);
        DeleteObject(w32_window->canvas_dib);
        w32_window->canvas_dc = 0;
        w32_window->canvas_dib = 0;
        window->canvas = 0;
    }
}

MU_StaticFunc void MU_WIN32_DrawCanvas(MU_Window *window) {
    if (window->canvas) {
        MU_Win32_Window *w32_window = (MU_Win32_Window *)window->platform;
        SelectObject(w32_window->canvas_dc, w32_window->canvas_dib);
        BitBlt(w32_window->handle_dc, 0, 0, window->size.x, window->size.y, w32_window->canvas_dc, 0, 0, SRCCOPY);
    }
}

MU_API void MU_ToggleFPSMode(MU_Window *window) {
    ShowCursor(window->is_fps_mode);
    window->is_fps_mode = !window->is_fps_mode;
}

MU_API void MU_DisableFPSMode(MU_Window *window) {
    if (window->is_fps_mode == true) MU_ToggleFPSMode(window);
}

MU_API void MU_EnableFPSMode(MU_Window *window) {
    if (window->is_fps_mode == false) MU_ToggleFPSMode(window);
}

MU_StaticFunc void MU__MemoryCopy(void *dst, const void *src, size_t size) {
    char *src8 = (char *)src;
    char *dst8 = (char *)dst;
    while (size--) *dst8++ = *src8++;
}

// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
MU_API void MU_ToggleFullscreen(MU_Window *window) {
    MU_Win32_Window *w32_window = (MU_Win32_Window *)window->platform;
    DWORD dwStyle = GetWindowLong((HWND)window->handle, GWL_STYLE);
    if (window->is_fullscreen == false) {
        MONITORINFO mi = {sizeof(mi)};
        if (GetWindowPlacement((HWND)window->handle, &w32_window->prev_placement) &&
            GetMonitorInfo(MonitorFromWindow((HWND)window->handle, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong((HWND)window->handle, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            BOOL result = SetWindowPos((HWND)window->handle, HWND_TOP,
                                       mi.rcMonitor.left, mi.rcMonitor.top,
                                       mi.rcMonitor.right - mi.rcMonitor.left,
                                       mi.rcMonitor.bottom - mi.rcMonitor.top,
                                       SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            if (result) window->is_fullscreen = true;
        }
    }
    else {
        SetWindowLong((HWND)window->handle, GWL_STYLE, w32_window->style);
        SetWindowPlacement((HWND)window->handle, &w32_window->prev_placement);
        BOOL result = SetWindowPos((HWND)window->handle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        if (result) window->is_fullscreen = false;
    }
}

MU_StaticFunc void MU_PushDroppedFile(MU_Context *mu, MU_Window *window, char *str, int size) {
    if (MU_DoesSizeFit(&mu->frame_arena, sizeof(MU_DroppedFile) + size)) {
        MU_DroppedFile *result = MU_PUSH_STRUCT(&mu->frame_arena, MU_DroppedFile);
        result->filename = (char *)MU_PushSize(&mu->frame_arena, size + 1);
        result->filename_size = size;
        MU__MemoryCopy(result->filename, str, size);
        result->filename[size] = 0;

        result->next = window->first_dropped_file;
        window->first_dropped_file = result;
    }
}

// Should be initialized before processing events
// Should be initialized before initializing opengl functions using GLAD
static MU_Context *MU_WIN32_ContextPointerForEventHandling = 0;
static LRESULT CALLBACK MU_WIN32_WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    MU_Context *mu = MU_WIN32_ContextPointerForEventHandling;
    MU_Win32 *w32 = (MU_Win32 *)mu->platform;

    MU_WIN32_UpdateFocusedWindowBasedOnHandle(mu, wnd);
    MU_Window *window = mu->window ? mu->window : 0;
    if (window) window->processed_events_this_frame += 1;

    (void)w32;
    switch (msg) {

        case WM_DROPFILES: {
            wchar_t buffer[512];
            char buffer8[1024];

            HDROP hdrop = (HDROP)wparam;
            int file_count = (int)DragQueryFileW(hdrop, 0xffffffff, NULL, 0);
            bool drop_failed = false;
            for (int i = 0; i < file_count; i += 1) {
                // UINT num_chars = DragQueryFileW(hdrop, i, NULL, 0) + 1;
                // WCHAR* buffer = (WCHAR*) _sapp_malloc_clear(num_chars * sizeof(WCHAR));
                UINT result = DragQueryFileW(hdrop, i, buffer, MU_ARRAY_SIZE(buffer));
                MU_ASSERT(result != 0);
                if (result) {
                    int64_t size = MU_CreateCharFromWidechar(buffer8, MU_ARRAY_SIZE(buffer8), buffer, MU_ARRAY_SIZE(buffer));
                    if (size != -1) {
                        MU_PushDroppedFile(mu, window, buffer8, (int)size);
                    }
                }
            }
            DragFinish(hdrop);
        } break;

        case WM_CLOSE: {
            // @! Make sure that focus works
            // @! We should find the window and make sure we inform it that the user clicked the close button
            PostQuitMessage(0);
            mu->quit = true;
        } break;

        case WM_LBUTTONDOWN: {
            SetCapture(wnd);
            if (window->change_cursor_on_mouse_hold) SetCursor(w32->cursor_hand);
            if (window->mouse.left.down == false)
                window->mouse.left.press = true;
            window->mouse.left.down = true;
        } break;

        case WM_LBUTTONUP: {
            ReleaseCapture();
            if (window->change_cursor_on_mouse_hold) SetCursor(w32->cursor_arrow);
            if (window->mouse.left.down == true)
                window->mouse.left.unpress = true;
            window->mouse.left.down = false;
        } break;

        case WM_RBUTTONDOWN: {
            SetCapture(wnd);
            if (window->mouse.right.down == false)
                window->mouse.right.press = true;
            window->mouse.right.down = true;
        } break;

        case WM_RBUTTONUP: {
            ReleaseCapture();
            if (window->mouse.right.down == true)
                window->mouse.right.unpress = true;
            window->mouse.right.down = false;
        } break;

        case WM_MBUTTONDOWN: {
            SetCapture(wnd);
            if (window->mouse.middle.down == false)
                window->mouse.middle.press = true;
            window->mouse.middle.down = true;
        } break;

        case WM_MBUTTONUP: {
            ReleaseCapture();
            if (window->mouse.middle.down == true)
                window->mouse.middle.unpress = true;
            window->mouse.middle.down = false;
        } break;

        case WM_MOUSEWHEEL: {
            if ((int)wparam > 0) {
                window->mouse.delta_wheel += 1.0f;
            }
            else {
                window->mouse.delta_wheel -= 1.0f;
            }
        } break;

        case WM_CHAR: {
            MU_UTF32Result encode = MU_UTF16ToUTF32((uint16_t *)&wparam, 2);
            if (encode.error) {
                MU__WriteChar32(window, (uint32_t)'?');
                MU__WriteChar8(window, "?", 1);
            }
            else {
                MU__WriteChar32(window, encode.out_str);
            }

            // Utf8 encode
            if (encode.error == false) {
                MU_UTF8Result encode8 = MU_UTF32ToUTF8(encode.out_str);
                if (encode8.error) {
                    MU__WriteChar8(window, "?", 1);
                }
                else {
                    MU__WriteChar8(window, encode8.out_str, encode8.len);
                }
            }
        } break;

        case WM_KEYUP:
        case WM_SYSKEYUP: {
            switch (wparam) {
                case VK_ESCAPE: MU__KeyUP(window, MU_KEY_ESCAPE); break;
                case VK_RETURN: MU__KeyUP(window, MU_KEY_ENTER); break;
                case VK_TAB: MU__KeyUP(window, MU_KEY_TAB); break;
                case VK_BACK: MU__KeyUP(window, MU_KEY_BACKSPACE); break;
                case VK_INSERT: MU__KeyUP(window, MU_KEY_INSERT); break;
                case VK_DELETE: MU__KeyUP(window, MU_KEY_DELETE); break;
                case VK_RIGHT: MU__KeyUP(window, MU_KEY_RIGHT); break;
                case VK_LEFT: MU__KeyUP(window, MU_KEY_LEFT); break;
                case VK_DOWN: MU__KeyUP(window, MU_KEY_DOWN); break;
                case VK_UP: MU__KeyUP(window, MU_KEY_UP); break;
                case VK_PRIOR: MU__KeyUP(window, MU_KEY_PAGE_UP); break;
                case VK_NEXT: MU__KeyUP(window, MU_KEY_PAGE_DOWN); break;
                case VK_END: MU__KeyUP(window, MU_KEY_HOME); break;
                case VK_HOME: MU__KeyUP(window, MU_KEY_END); break;
                case VK_F1: MU__KeyUP(window, MU_KEY_F1); break;
                case VK_F2: MU__KeyUP(window, MU_KEY_F2); break;
                case VK_F3: MU__KeyUP(window, MU_KEY_F3); break;
                case VK_F4: MU__KeyUP(window, MU_KEY_F4); break;
                case VK_F5: MU__KeyUP(window, MU_KEY_F5); break;
                case VK_F6: MU__KeyUP(window, MU_KEY_F6); break;
                case VK_F7: MU__KeyUP(window, MU_KEY_F7); break;
                case VK_F8: MU__KeyUP(window, MU_KEY_F8); break;
                case VK_F9: MU__KeyUP(window, MU_KEY_F9); break;
                case VK_F10: MU__KeyUP(window, MU_KEY_F10); break;
                case VK_F11: MU__KeyUP(window, MU_KEY_F11); break;
                case VK_F12: MU__KeyUP(window, MU_KEY_F12); break;
                case VK_SPACE: MU__KeyUP(window, MU_KEY_SPACE); break;
                case VK_OEM_PLUS: MU__KeyUP(window, MU_KEY_PLUS); break;
                case VK_OEM_COMMA: MU__KeyUP(window, MU_KEY_COMMA); break;
                case VK_OEM_MINUS: MU__KeyUP(window, MU_KEY_MINUS); break;
                case VK_OEM_PERIOD: MU__KeyUP(window, MU_KEY_PERIOD); break;
                case '0': MU__KeyUP(window, MU_KEY_0); break;
                case '1': MU__KeyUP(window, MU_KEY_1); break;
                case '2': MU__KeyUP(window, MU_KEY_2); break;
                case '3': MU__KeyUP(window, MU_KEY_3); break;
                case '4': MU__KeyUP(window, MU_KEY_4); break;
                case '5': MU__KeyUP(window, MU_KEY_5); break;
                case '6': MU__KeyUP(window, MU_KEY_6); break;
                case '7': MU__KeyUP(window, MU_KEY_7); break;
                case '8': MU__KeyUP(window, MU_KEY_8); break;
                case '9': MU__KeyUP(window, MU_KEY_9); break;
                case ';': MU__KeyUP(window, MU_KEY_SEMICOLON); break;
                case '=': MU__KeyUP(window, MU_KEY_EQUAL); break;
                case 'A': MU__KeyUP(window, MU_KEY_A); break;
                case 'B': MU__KeyUP(window, MU_KEY_B); break;
                case 'C': MU__KeyUP(window, MU_KEY_C); break;
                case 'D': MU__KeyUP(window, MU_KEY_D); break;
                case 'E': MU__KeyUP(window, MU_KEY_E); break;
                case 'F': MU__KeyUP(window, MU_KEY_F); break;
                case 'G': MU__KeyUP(window, MU_KEY_G); break;
                case 'H': MU__KeyUP(window, MU_KEY_H); break;
                case 'I': MU__KeyUP(window, MU_KEY_I); break;
                case 'J': MU__KeyUP(window, MU_KEY_J); break;
                case 'K': MU__KeyUP(window, MU_KEY_K); break;
                case 'L': MU__KeyUP(window, MU_KEY_L); break;
                case 'M': MU__KeyUP(window, MU_KEY_M); break;
                case 'N': MU__KeyUP(window, MU_KEY_N); break;
                case 'O': MU__KeyUP(window, MU_KEY_O); break;
                case 'P': MU__KeyUP(window, MU_KEY_P); break;
                case 'Q': MU__KeyUP(window, MU_KEY_Q); break;
                case 'R': MU__KeyUP(window, MU_KEY_R); break;
                case 'S': MU__KeyUP(window, MU_KEY_S); break;
                case 'T': MU__KeyUP(window, MU_KEY_T); break;
                case 'U': MU__KeyUP(window, MU_KEY_U); break;
                case 'V': MU__KeyUP(window, MU_KEY_V); break;
                case 'W': MU__KeyUP(window, MU_KEY_W); break;
                case 'X': MU__KeyUP(window, MU_KEY_X); break;
                case 'Y': MU__KeyUP(window, MU_KEY_Y); break;
                case 'Z': MU__KeyUP(window, MU_KEY_Z); break;
                case VK_F13: MU__KeyUP(window, MU_KEY_F13); break;
                case VK_F14: MU__KeyUP(window, MU_KEY_F14); break;
                case VK_F15: MU__KeyUP(window, MU_KEY_F15); break;
                case VK_F16: MU__KeyUP(window, MU_KEY_F16); break;
                case VK_F17: MU__KeyUP(window, MU_KEY_F17); break;
                case VK_F18: MU__KeyUP(window, MU_KEY_F18); break;
                case VK_F19: MU__KeyUP(window, MU_KEY_F19); break;
                case VK_F20: MU__KeyUP(window, MU_KEY_F20); break;
                case VK_F21: MU__KeyUP(window, MU_KEY_F21); break;
                case VK_F22: MU__KeyUP(window, MU_KEY_F22); break;
                case VK_F23: MU__KeyUP(window, MU_KEY_F23); break;
                case VK_F24: MU__KeyUP(window, MU_KEY_F24); break;
                case 0x60: MU__KeyUP(window, MU_KEY_KP_0); break;
                case 0x61: MU__KeyUP(window, MU_KEY_KP_1); break;
                case 0x62: MU__KeyUP(window, MU_KEY_KP_2); break;
                case 0x63: MU__KeyUP(window, MU_KEY_KP_3); break;
                case 0x64: MU__KeyUP(window, MU_KEY_KP_4); break;
                case 0x65: MU__KeyUP(window, MU_KEY_KP_5); break;
                case 0x66: MU__KeyUP(window, MU_KEY_KP_6); break;
                case 0x67: MU__KeyUP(window, MU_KEY_KP_7); break;
                case 0x68: MU__KeyUP(window, MU_KEY_KP_8); break;
                case 0x69: MU__KeyUP(window, MU_KEY_KP_9); break;
                case VK_DECIMAL: MU__KeyUP(window, MU_KEY_KP_DECIMAL); break;
                case VK_DIVIDE: MU__KeyUP(window, MU_KEY_KP_DIVIDE); break;
                case VK_MULTIPLY: MU__KeyUP(window, MU_KEY_KP_MULTIPLY); break;
                case VK_SUBTRACT: MU__KeyUP(window, MU_KEY_KP_SUBTRACT); break;
                case VK_ADD: MU__KeyUP(window, MU_KEY_KP_ADD); break;
                case VK_LMENU: MU__KeyUP(window, MU_KEY_LEFT_ALT); break;
                case VK_LWIN: MU__KeyUP(window, MU_KEY_LEFT_SUPER); break;
                case VK_CONTROL: MU__KeyUP(window, MU_KEY_CONTROL); break;
                case VK_SHIFT: MU__KeyUP(window, MU_KEY_SHIFT); break;
                case VK_LSHIFT: MU__KeyUP(window, MU_KEY_LEFT_SHIFT); break;
                case VK_LCONTROL: MU__KeyUP(window, MU_KEY_LEFT_CONTROL); break;
                case VK_RSHIFT: MU__KeyUP(window, MU_KEY_RIGHT_SHIFT); break;
                case VK_RCONTROL: MU__KeyUP(window, MU_KEY_RIGHT_CONTROL); break;
                case VK_RMENU: MU__KeyUP(window, MU_KEY_RIGHT_ALT); break;
                case VK_RWIN: MU__KeyUP(window, MU_KEY_RIGHT_SUPER); break;
                case VK_CAPITAL: MU__KeyUP(window, MU_KEY_CAPS_LOCK); break;
                case VK_SCROLL: MU__KeyUP(window, MU_KEY_SCROLL_LOCK); break;
                case VK_NUMLOCK: MU__KeyUP(window, MU_KEY_NUM_LOCK); break;
                case VK_SNAPSHOT: MU__KeyUP(window, MU_KEY_PRINT_SCREEN); break;
                case VK_PAUSE: MU__KeyUP(window, MU_KEY_PAUSE); break;
            }
        } break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            switch (wparam) {
                case VK_ESCAPE: MU__KeyDown(window, MU_KEY_ESCAPE); break;
                case VK_RETURN: MU__KeyDown(window, MU_KEY_ENTER); break;
                case VK_TAB: MU__KeyDown(window, MU_KEY_TAB); break;
                case VK_BACK: MU__KeyDown(window, MU_KEY_BACKSPACE); break;
                case VK_INSERT: MU__KeyDown(window, MU_KEY_INSERT); break;
                case VK_DELETE: MU__KeyDown(window, MU_KEY_DELETE); break;
                case VK_RIGHT: MU__KeyDown(window, MU_KEY_RIGHT); break;
                case VK_LEFT: MU__KeyDown(window, MU_KEY_LEFT); break;
                case VK_DOWN: MU__KeyDown(window, MU_KEY_DOWN); break;
                case VK_UP: MU__KeyDown(window, MU_KEY_UP); break;
                case VK_PRIOR: MU__KeyDown(window, MU_KEY_PAGE_UP); break;
                case VK_NEXT: MU__KeyDown(window, MU_KEY_PAGE_DOWN); break;
                case VK_END: MU__KeyDown(window, MU_KEY_HOME); break;
                case VK_HOME: MU__KeyDown(window, MU_KEY_END); break;
                case VK_F1: MU__KeyDown(window, MU_KEY_F1); break;
                case VK_F2: MU__KeyDown(window, MU_KEY_F2); break;
                case VK_F3: MU__KeyDown(window, MU_KEY_F3); break;
                case VK_F4: MU__KeyDown(window, MU_KEY_F4); break;
                case VK_F5: MU__KeyDown(window, MU_KEY_F5); break;
                case VK_F6: MU__KeyDown(window, MU_KEY_F6); break;
                case VK_F7: MU__KeyDown(window, MU_KEY_F7); break;
                case VK_F8: MU__KeyDown(window, MU_KEY_F8); break;
                case VK_F9: MU__KeyDown(window, MU_KEY_F9); break;
                case VK_F10: MU__KeyDown(window, MU_KEY_F10); break;
                case VK_F11: MU__KeyDown(window, MU_KEY_F11); break;
                case VK_F12: MU__KeyDown(window, MU_KEY_F12); break;
                case VK_SPACE: MU__KeyDown(window, MU_KEY_SPACE); break;
                case VK_OEM_PLUS: MU__KeyDown(window, MU_KEY_PLUS); break;
                case VK_OEM_COMMA: MU__KeyDown(window, MU_KEY_COMMA); break;
                case VK_OEM_MINUS: MU__KeyDown(window, MU_KEY_MINUS); break;
                case VK_OEM_PERIOD: MU__KeyDown(window, MU_KEY_PERIOD); break;
                case '0': MU__KeyDown(window, MU_KEY_0); break;
                case '1': MU__KeyDown(window, MU_KEY_1); break;
                case '2': MU__KeyDown(window, MU_KEY_2); break;
                case '3': MU__KeyDown(window, MU_KEY_3); break;
                case '4': MU__KeyDown(window, MU_KEY_4); break;
                case '5': MU__KeyDown(window, MU_KEY_5); break;
                case '6': MU__KeyDown(window, MU_KEY_6); break;
                case '7': MU__KeyDown(window, MU_KEY_7); break;
                case '8': MU__KeyDown(window, MU_KEY_8); break;
                case '9': MU__KeyDown(window, MU_KEY_9); break;
                case ';': MU__KeyDown(window, MU_KEY_SEMICOLON); break;
                case '=': MU__KeyDown(window, MU_KEY_EQUAL); break;
                case 'A': MU__KeyDown(window, MU_KEY_A); break;
                case 'B': MU__KeyDown(window, MU_KEY_B); break;
                case 'C': MU__KeyDown(window, MU_KEY_C); break;
                case 'D': MU__KeyDown(window, MU_KEY_D); break;
                case 'E': MU__KeyDown(window, MU_KEY_E); break;
                case 'F': MU__KeyDown(window, MU_KEY_F); break;
                case 'G': MU__KeyDown(window, MU_KEY_G); break;
                case 'H': MU__KeyDown(window, MU_KEY_H); break;
                case 'I': MU__KeyDown(window, MU_KEY_I); break;
                case 'J': MU__KeyDown(window, MU_KEY_J); break;
                case 'K': MU__KeyDown(window, MU_KEY_K); break;
                case 'L': MU__KeyDown(window, MU_KEY_L); break;
                case 'M': MU__KeyDown(window, MU_KEY_M); break;
                case 'N': MU__KeyDown(window, MU_KEY_N); break;
                case 'O': MU__KeyDown(window, MU_KEY_O); break;
                case 'P': MU__KeyDown(window, MU_KEY_P); break;
                case 'Q': MU__KeyDown(window, MU_KEY_Q); break;
                case 'R': MU__KeyDown(window, MU_KEY_R); break;
                case 'S': MU__KeyDown(window, MU_KEY_S); break;
                case 'T': MU__KeyDown(window, MU_KEY_T); break;
                case 'U': MU__KeyDown(window, MU_KEY_U); break;
                case 'V': MU__KeyDown(window, MU_KEY_V); break;
                case 'W': MU__KeyDown(window, MU_KEY_W); break;
                case 'X': MU__KeyDown(window, MU_KEY_X); break;
                case 'Y': MU__KeyDown(window, MU_KEY_Y); break;
                case 'Z': MU__KeyDown(window, MU_KEY_Z); break;
                case VK_F13: MU__KeyDown(window, MU_KEY_F13); break;
                case VK_F14: MU__KeyDown(window, MU_KEY_F14); break;
                case VK_F15: MU__KeyDown(window, MU_KEY_F15); break;
                case VK_F16: MU__KeyDown(window, MU_KEY_F16); break;
                case VK_F17: MU__KeyDown(window, MU_KEY_F17); break;
                case VK_F18: MU__KeyDown(window, MU_KEY_F18); break;
                case VK_F19: MU__KeyDown(window, MU_KEY_F19); break;
                case VK_F20: MU__KeyDown(window, MU_KEY_F20); break;
                case VK_F21: MU__KeyDown(window, MU_KEY_F21); break;
                case VK_F22: MU__KeyDown(window, MU_KEY_F22); break;
                case VK_F23: MU__KeyDown(window, MU_KEY_F23); break;
                case VK_F24: MU__KeyDown(window, MU_KEY_F24); break;
                case 0x60: MU__KeyDown(window, MU_KEY_KP_0); break;
                case 0x61: MU__KeyDown(window, MU_KEY_KP_1); break;
                case 0x62: MU__KeyDown(window, MU_KEY_KP_2); break;
                case 0x63: MU__KeyDown(window, MU_KEY_KP_3); break;
                case 0x64: MU__KeyDown(window, MU_KEY_KP_4); break;
                case 0x65: MU__KeyDown(window, MU_KEY_KP_5); break;
                case 0x66: MU__KeyDown(window, MU_KEY_KP_6); break;
                case 0x67: MU__KeyDown(window, MU_KEY_KP_7); break;
                case 0x68: MU__KeyDown(window, MU_KEY_KP_8); break;
                case 0x69: MU__KeyDown(window, MU_KEY_KP_9); break;
                case VK_CONTROL: MU__KeyDown(window, MU_KEY_CONTROL); break;
                case VK_SHIFT: MU__KeyDown(window, MU_KEY_SHIFT); break;
                case VK_DECIMAL: MU__KeyDown(window, MU_KEY_KP_DECIMAL); break;
                case VK_DIVIDE: MU__KeyDown(window, MU_KEY_KP_DIVIDE); break;
                case VK_MULTIPLY: MU__KeyDown(window, MU_KEY_KP_MULTIPLY); break;
                case VK_SUBTRACT: MU__KeyDown(window, MU_KEY_KP_SUBTRACT); break;
                case VK_ADD: MU__KeyDown(window, MU_KEY_KP_ADD); break;
                case VK_LSHIFT: MU__KeyDown(window, MU_KEY_LEFT_SHIFT); break;
                case VK_LCONTROL: MU__KeyDown(window, MU_KEY_LEFT_CONTROL); break;
                case VK_LMENU: MU__KeyDown(window, MU_KEY_LEFT_ALT); break;
                case VK_LWIN: MU__KeyDown(window, MU_KEY_LEFT_SUPER); break;
                case VK_RSHIFT: MU__KeyDown(window, MU_KEY_RIGHT_SHIFT); break;
                case VK_RCONTROL: MU__KeyDown(window, MU_KEY_RIGHT_CONTROL); break;
                case VK_RMENU: MU__KeyDown(window, MU_KEY_RIGHT_ALT); break;
                case VK_RWIN: MU__KeyDown(window, MU_KEY_RIGHT_SUPER); break;
                case VK_CAPITAL: MU__KeyDown(window, MU_KEY_CAPS_LOCK); break;
                case VK_SCROLL: MU__KeyDown(window, MU_KEY_SCROLL_LOCK); break;
                case VK_NUMLOCK: MU__KeyDown(window, MU_KEY_NUM_LOCK); break;
                case VK_SNAPSHOT: MU__KeyDown(window, MU_KEY_PRINT_SCREEN); break;
                case VK_PAUSE: MU__KeyDown(window, MU_KEY_PAUSE); break;
            }
        } break;

        default: {
            return DefWindowProcW(wnd, msg, wparam, lparam);
        }
    }
    return 0;
}

MU_API void MU_Init(MU_Context *mu, MU_Params params, size_t len) {
    MU_ASSERT(params.memory && params.cap && "Expected any kind of memory");

    MU__ZeroMemory(mu, sizeof(*mu));
    mu->perm_arena.memory = (char *)params.memory;
    mu->perm_arena.cap = params.cap;
    mu->perm_arena.len = len;
    MU_WIN32_ContextPointerForEventHandling = mu;

    mu->platform = MU_PUSH_STRUCT(&mu->perm_arena, MU_Win32);
    MU_Win32 *w32 = (MU_Win32 *)mu->platform;

    mu->frame_arena.cap = (mu->perm_arena.cap - mu->perm_arena.len) / 2;
    mu->frame_arena.memory = (char *)MU_PushSize(&mu->perm_arena, mu->frame_arena.cap);

    mu->time.delta = params.delta_time == 0.0 ? 0.0166666 : params.delta_time;

    mu->sound.callback = params.sound_callback;
    mu->params = params;

    if (mu->sound.callback) {
        MU_WIN32_InitWasapi(mu);
        MU_ASSERT(mu->sound.initialized);
    }

    typedef enum MU_PROCESS_DPI_AWARENESS {
        MU_PROCESS_DPI_UNAWARE = 0,
        MU_PROCESS_SYSTEM_DPI_AWARE = 1,
        MU_PROCESS_PER_MONITOR_DPI_AWARE = 2
    } MU_PROCESS_DPI_AWARENESS;
    typedef unsigned MU_TimeBeginPeriod(unsigned);
    typedef HRESULT MU_SetProcessDpiAwareness(MU_PROCESS_DPI_AWARENESS);

    HMODULE shcore = LoadLibraryA("Shcore.dll");
    if (shcore) {
        MU_SetProcessDpiAwareness *set_dpi_awr = (MU_SetProcessDpiAwareness *)GetProcAddress(shcore, "SetProcessDpiAwareness");
        if (set_dpi_awr) {
            HRESULT hr = set_dpi_awr(MU_PROCESS_PER_MONITOR_DPI_AWARE);
            MU_ASSERT(SUCCEEDED(hr) && "Failed to set dpi awareness");
        }
    }

    HMODULE winmm = LoadLibraryA("winmm.dll");
    if (winmm) {
        MU_TimeBeginPeriod *timeBeginPeriod = (MU_TimeBeginPeriod *)GetProcAddress(winmm, "timeBeginPeriod");
        if (timeBeginPeriod) {
            if (timeBeginPeriod(1) == 0) {
                w32->good_scheduling = true;
            }
        }
    }

    WNDCLASSW wc;
    {
        MU__ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = MU_WIN32_WindowProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = L"Multimedia_Start";
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hIcon = NULL; // LoadIcon(wc.hInstance, IDI_APPLICATION);
        wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        MU_ASSERT_CODE(ATOM result =)
        RegisterClassW(&wc);
        MU_ASSERT(result != 0);
        w32->wc = wc;
    }

    mu->primary_monitor_size.x = GetSystemMetrics(SM_CXSCREEN);
    mu->primary_monitor_size.y = GetSystemMetrics(SM_CYSCREEN);

    w32->cursor_hand = LoadCursor(0, IDC_SIZEALL);
    w32->cursor_arrow = LoadCursor(0, IDC_ARROW);

    mu->time.app_start = MU_GetTime();
    mu->first_frame = true;
}

MU_StaticFunc void MU_UpdateWindowState(MU_Window *window) {
    MU_Win32_Window *w32_window = (MU_Win32_Window *)window->platform;

    UINT dpi = GetDpiForWindow((HWND)window->handle);
    MU_ASSERT(dpi != 0 && "Failed to get dpi for window");
    window->dpi_scale = (float)dpi / 96.f;

    MU_Int2 size;
    MU_WIN32_GetWindowPos((HWND)window->handle, &window->pos.x, &window->pos.y);
    MU_Win32_GetWindowSize((HWND)window->handle, &size.x, &size.y);

    if (window->canvas_enabled == false || window->size.x != size.x || window->size.y != size.y) {
        MU_WIN32_DestroyCanvas(window);
    }

    window->size = size;
    window->sizef.x = (float)window->size.x;
    window->sizef.y = (float)window->size.y;

    window->posf.x = (float)window->pos.x;
    window->posf.y = (float)window->pos.y;

    if (window->canvas_enabled && window->canvas == 0) {
        MU_WIN32_CreateCanvas(window);
    }
}

MU_API MU_Window *MU_AddWindow(MU_Context *mu, MU_Window_Params params) {
    MU_Window *window = MU_PUSH_STRUCT(&mu->perm_arena, MU_Window);
    MU_InitWindow(mu, window, params);
    return window;
}

MU_API void MU_InitWindow(MU_Context *mu, MU_Window *window, MU_Window_Params params) {
    window->platform = MU_PUSH_STRUCT(&mu->perm_arena, MU_Win32_Window);

    MU_Win32 *w32 = (MU_Win32 *)mu->platform;
    MU_Win32_Window *w32_window = (MU_Win32_Window *)window->platform;

    if (params.pos.x == 0) params.pos.x = (int)((double)mu->primary_monitor_size.x * 0.1);
    if (params.pos.y == 0) params.pos.y = (int)((double)mu->primary_monitor_size.y * 0.1);
    if (params.size.x == 0) params.size.x = (int)((double)mu->primary_monitor_size.x * 0.8);
    if (params.size.y == 0) params.size.y = (int)((double)mu->primary_monitor_size.y * 0.8);
    window->canvas_enabled = params.enable_canvas;

    w32_window->style = WS_OVERLAPPEDWINDOW;
    if (!params.resizable) {
        w32_window->style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    }
    if (params.borderless) {
        w32_window->style = WS_POPUP | WS_VISIBLE | WS_SYSMENU;
    }

    RECT window_rect;
    window_rect.left = (LONG)params.pos.x;
    window_rect.top = (LONG)params.pos.y;
    window_rect.right = (LONG)params.size.x + window_rect.left;
    window_rect.bottom = (LONG)params.size.y + window_rect.top;
    AdjustWindowRectEx(&window_rect, w32_window->style, false, 0);

    HWND handle = CreateWindowW(w32->wc.lpszClassName, L"Zzz... Window, hello!", w32_window->style, window_rect.left, window_rect.top, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, NULL, NULL, w32->wc.hInstance, NULL);
    MU_ASSERT(handle);

    window->handle = handle;
    w32_window->handle_dc = GetDC(handle);
    MU_ASSERT(w32_window->handle_dc);

    DragAcceptFiles(handle, TRUE);

    MU_WIN32_TryToInitGLContextForWindow(mu, w32_window);

    ShowWindow(handle, SW_SHOW);
    MU_UpdateWindowState(window);
    MU_STACK_ADD(mu->all_windows, window);
    MU_WIN32_UpdateFocusedWindow(mu);
}

MU_API MU_Context *MU_Start(MU_Params params) {
    // Bootstrap the context from user memory
    // If the user didnt provide memory, allocate it ourselves
    if (!params.memory) {
        HANDLE process_heap = GetProcessHeap();
        params.cap = MU_DEFAULT_MEMORY_SIZE;
        params.memory = HeapAlloc(process_heap, 0, params.cap);
        MU_ASSERT(params.memory);
    }
    MU_Context *mu = (MU_Context *)params.memory;
    MU_Init(mu, params, sizeof(MU_Context));
    mu->window = MU_AddWindow(mu, params.window);

    return mu;
}

MU_API bool MU_Update(MU_Context *mu) {
    MU_Win32 *w32 = (MU_Win32 *)mu->platform;

    // Since this is meant to be called in while loop
    // first MU_Update happens before first frame
    // therfore start of second MU_Update is end of first frame
    mu->_MU_Update_count += 1;
    if (mu->_MU_Update_count == 2) mu->first_frame = false;
    mu->frame_arena.len = 0;

    MU_WIN32_UpdateFocusedWindow(mu);
    for (MU_Window *it = mu->all_windows; it; it = it->next) {
        if (it->should_render == true && mu->first_frame == false && mu->opengl_initialized) {
            MU_Win32_Window *w32_window = (MU_Win32_Window *)it->platform;
            MU_ASSERT_CODE(BOOL result =)
            SwapBuffers(w32_window->handle_dc);
            MU_ASSERT(result);
        }
        it->should_render = true;

        it->first_dropped_file = 0;
        MU_WIN32_DrawCanvas(it);
        it->processed_events_this_frame = 0;
        it->user_text8_count = 0;
        it->user_text32_count = 0;
        it->mouse.delta_wheel = 0.0;
        it->mouse.left.press = 0;
        it->mouse.right.press = 0;
        it->mouse.middle.press = 0;
        it->mouse.left.unpress = 0;
        it->mouse.right.unpress = 0;
        it->mouse.middle.unpress = 0;
        for (int i = 0; i < MU_KEY_COUNT; i += 1) {
            it->key[i].press = 0;
            it->key[i].unpress = 0;
            it->key[i].raw_press = 0;
        }
    }

    MSG msg;
    MU_WIN32_ContextPointerForEventHandling = mu;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    for (MU_Window *it = mu->all_windows; it; it = it->next) {
        MU_UpdateWindowState(it);
    }

    mu->window->user_text8[mu->window->user_text8_count] = 0;
    mu->window->user_text32[mu->window->user_text32_count] = 0;

    MU_Win32_Window *w32_window = (MU_Win32_Window *)mu->window->platform;
    HWND focused_window = GetFocus();
    mu->window->is_focused = focused_window == (HWND)mu->window->handle;

    // We only need to update the mouse position of a currently focused window?
    {

        MU_Int2 mouse_pos = MU_WIN32_GetMousePositionInverted((HWND)mu->window->handle, mu->window->size.y);

        if (mu->window->is_focused) {
            if (mu->first_frame == false) {
                mu->window->mouse.delta_pos.x = mouse_pos.x - mu->window->mouse.pos.x;
                mu->window->mouse.delta_pos.y = mouse_pos.y - mu->window->mouse.pos.y;
                mu->window->mouse.delta_pos_normalized.x = (float)mu->window->mouse.delta_pos.x / (float)mu->window->size.x;
                mu->window->mouse.delta_pos_normalized.y = (float)mu->window->mouse.delta_pos.y / (float)mu->window->size.y;
            }
            if (mu->window->is_fps_mode) {
                SetCursorPos(mu->window->size.x / 2, mu->window->size.y / 2);
                mouse_pos = MU_WIN32_GetMousePositionInverted((HWND)mu->window->handle, mu->window->size.y);
            }
        }
        mu->window->mouse.pos = mouse_pos;
        mu->window->mouse.posf.x = (float)mouse_pos.x;
        mu->window->mouse.posf.y = (float)mouse_pos.y;
    }

    // Timming
    if (mu->first_frame == false) {
        mu->time.update = MU_GetTime() - mu->time.frame_start;
        if (mu->time.update < mu->time.delta) {
            mu->consecutive_missed_frames = 0;

            // Try to use the Sleep, if we dont have good scheduler priority
            // then we can miss framerate so need to busy loop instead
            if (w32->good_scheduling) {
                double time_to_sleep = mu->time.delta - mu->time.update;
                double time_to_sleep_in_ms = time_to_sleep * 1000.0 - 1;
                if (time_to_sleep > 0.0) {
                    DWORD time_to_sleep_uint = (DWORD)time_to_sleep_in_ms;
                    if (time_to_sleep_uint) {
                        Sleep(time_to_sleep_uint);
                    }
                }
            }

            // Busy loop if we dont have good scheduling
            // or we woke up early
            double update_time = MU_GetTime() - mu->time.frame_start;
            while (update_time < mu->time.delta) {
                update_time = MU_GetTime() - mu->time.frame_start;
            }
        }
        else {
            mu->consecutive_missed_frames += 1;
            mu->total_missed_frames += 1;
        }

        mu->frame += 1;
        mu->time.update_total = MU_GetTime() - mu->time.frame_start;
        mu->time.total += mu->time.delta;
    }
    mu->time.frame_start = MU_GetTime();

    mu->time.deltaf = (float)mu->time.delta;
    mu->time.totalf = (float)mu->time.total;

    return !mu->quit;
}

//
// Opengl context setup
//
// @! Cleanup OpenGL - Should the user be cappable of detecting that opengl couldnt load?
// Should the layer automatically downscale?
// Should the layer inform and allow for a response?
/*
    MU_Context *mu = MU_Start((MU_Params){
        .enable_opengl = true,
    });
    if (mu->opengl_initialized == false) {
        mu_opengl_try_initializng_context_for_window(mu->window, 3, 3);
    }
    if (mu->opengl_initialized == false) {
        // directx
    }


    */

// Symbols taken from GLFW
//
// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on Nvidia Optimus systems
// with up-to-date drivers
//
__declspec(dllexport) DWORD NvOptimusEnablement = 1;

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on AMD PowerXpress systems
// with up-to-date drivers
//
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

typedef HGLRC MU_wglCreateContext(HDC unnamedParam1);
typedef BOOL MU_wglMakeCurrent(HDC unnamedParam1, HGLRC unnamedParam2);
typedef BOOL MU_wglDeleteContext(HGLRC unnamedParam1);
HGLRC(*mu_wglCreateContext)
(HDC unnamedParam1);
BOOL(*mu_wglMakeCurrent)
(HDC unnamedParam1, HGLRC unnamedParam2);
BOOL(*mu_wglDeleteContext)
(HGLRC unnamedParam1);

typedef const char *MU_wglGetExtensionsStringARB(HDC hdc);
typedef BOOL MU_wglChoosePixelFormatARB(HDC hdc, const int *piAttribIList, const float *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC MU_wglCreateContextAttribsARB(HDC hDC, HGLRC hshareContext, const int *attribList);
typedef BOOL MU_wglSwapIntervalEXT(int interval);
MU_wglChoosePixelFormatARB *wglChoosePixelFormatARB;
MU_wglCreateContextAttribsARB *wglCreateContextAttribsARB;
MU_wglSwapIntervalEXT *wglSwapIntervalEXT;

    #define WGL_DRAW_TO_WINDOW_ARB 0x2001
    #define WGL_SUPPORT_OPENGL_ARB 0x2010
    #define WGL_DOUBLE_BUFFER_ARB 0x2011
    #define WGL_PIXEL_TYPE_ARB 0x2013
    #define WGL_TYPE_RGBA_ARB 0x202B
    #define WGL_COLOR_BITS_ARB 0x2014
    #define WGL_DEPTH_BITS_ARB 0x2022
    #define WGL_STENCIL_BITS_ARB 0x2023

    #define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
    #define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
    #define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
    #define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
    #define WGL_CONTEXT_FLAGS_ARB 0x2094
    #define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001

    #define WGL_SAMPLE_BUFFERS_ARB 0x2041
    #define WGL_SAMPLES_ARB 0x2042

//
// Below loading part largely taken from github gist of Martins Mozeiko
//

// compares src string with dstlen characters from dst, returns 1 if they are equal, 0 if not
MU_StaticFunc int MU__AreStringsEqual(const char *src, const char *dst, size_t dstlen) {
    while (*src && dstlen-- && *dst) {
        if (*src++ != *dst++) {
            return 0;
        }
    }

    return (dstlen && *src == *dst) || (!dstlen && *src == 0);
}

MU_StaticFunc void *MU_Win32_GLGetWindowProcAddressForGlad(const char *proc) {
    MU_Win32 *w32 = (MU_Win32 *)MU_WIN32_ContextPointerForEventHandling->platform;
    void *func;

    func = w32->wgl_get_proc_address(proc);
    if (!func) {
        func = GetProcAddress(w32->opengl32, proc);
    }
    return func;
}

MU_StaticFunc void MU_WIN32_GetWGLFunctions(MU_Context *mu) {
    MU_Win32 *w32 = (MU_Win32 *)mu->platform;
    HMODULE opengl32 = LoadLibraryA("opengl32");
    MU_ASSERT(opengl32);
    if (opengl32) {
        w32->opengl32 = opengl32;
        w32->wgl_get_proc_address = (MU_glGetProcAddress *)GetProcAddress(opengl32, "wglGetProcAddress");
        mu->gl_get_proc_address = MU_Win32_GLGetWindowProcAddressForGlad;
        mu_wglCreateContext = (MU_wglCreateContext *)GetProcAddress(opengl32, "wglCreateContext");
        mu_wglMakeCurrent = (MU_wglMakeCurrent *)GetProcAddress(opengl32, "wglMakeCurrent");
        mu_wglDeleteContext = (MU_wglDeleteContext *)GetProcAddress(opengl32, "wglDeleteContext");
    }
    if (opengl32 == NULL || mu_wglCreateContext == NULL || mu->gl_get_proc_address == NULL || mu_wglMakeCurrent == NULL || mu_wglDeleteContext == NULL) {
        MU_ASSERT(!"Failed to load Opengl wgl functions from opengl32.lib");
        return;
    }

    // to get WGL functions we need valid GL context, so create dummy window for dummy GL contetx
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL, NULL);
    MU_ASSERT(dummy && "Failed to create dummy window");

    HDC dc = GetDC(dummy);
    MU_ASSERT(dc && "Failed to get device context for dummy window");

    PIXELFORMATDESCRIPTOR desc;
    MU__ZeroMemory(&desc, sizeof(desc));
    {
        desc.nSize = sizeof(desc);
        desc.nVersion = 1;
        desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        desc.iPixelType = PFD_TYPE_RGBA;
        desc.cColorBits = 24;
    };

    int format = ChoosePixelFormat(dc, &desc);
    if (!format) {
        MU_ASSERT(!"Cannot choose OpenGL pixel format for dummy window!");
    }

    int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
    MU_ASSERT(ok && "Failed to describe OpenGL pixel format");

    // reason to create dummy window is that SetPixelFormat can be called only once for the window
    if (!SetPixelFormat(dc, format, &desc)) {
        MU_ASSERT(!"Cannot set OpenGL pixel format for dummy window!");
    }

    HGLRC rc = mu_wglCreateContext(dc);
    MU_ASSERT(rc && "Failed to create OpenGL context for dummy window");

    ok = mu_wglMakeCurrent(dc, rc);
    MU_ASSERT(ok && "Failed to make current OpenGL context for dummy window");

    // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_extensions_string.txt
    MU_wglGetExtensionsStringARB *wglGetExtensionsStringARB = (MU_wglGetExtensionsStringARB *)mu->gl_get_proc_address("wglGetExtensionsStringARB");
    if (!wglGetExtensionsStringARB) {
        MU_ASSERT(!"OpenGL does not support WGL_ARB_extensions_string extension!");
    }

    const char *ext = wglGetExtensionsStringARB(dc);
    MU_ASSERT(ext && "Failed to get OpenGL WGL extension string");

    const char *start = ext;
    for (;;) {
        while (*ext != 0 && *ext != ' ') {
            ext++;
        }
        size_t length = ext - start;
        if (MU__AreStringsEqual("WGL_ARB_pixel_format", start, length)) {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
            wglChoosePixelFormatARB = (MU_wglChoosePixelFormatARB *)mu->gl_get_proc_address("wglChoosePixelFormatARB");
        }
        else if (MU__AreStringsEqual("WGL_ARB_create_context", start, length)) {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
            wglCreateContextAttribsARB = (MU_wglCreateContextAttribsARB *)mu->gl_get_proc_address("wglCreateContextAttribsARB");
        }
        else if (MU__AreStringsEqual("WGL_EXT_swap_control", start, length)) {
            // https://www.khronos.org/registry/OpenGL/extensions/EXT/WGL_EXT_swap_control.txt
            wglSwapIntervalEXT = (MU_wglSwapIntervalEXT *)mu->gl_get_proc_address("wglSwapIntervalEXT");
        }

        if (*ext == 0) {
            break;
        }

        ext++;
        start = ext;
    }

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB || !wglSwapIntervalEXT) {
        MU_ASSERT(!"OpenGL does not support required WGL extensions for modern context!");
    }

    mu_wglMakeCurrent(NULL, NULL);
    mu_wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);

    mu->opengl_initialized = true;
}

MU_StaticFunc void MU_WIN32_TryToInitGLContextForWindow(MU_Context *mu, MU_Win32_Window *w32_window) {
    if (mu->opengl_initialized == false && mu->params.enable_opengl) {
        MU_WIN32_GetWGLFunctions(mu);
        if (mu->opengl_initialized) {
            mu->opengl_major = mu->params.opengl_major ? mu->params.opengl_major : 4;
            mu->opengl_minor = mu->params.opengl_minor ? mu->params.opengl_minor : 5;
        }
    }

    if (mu->opengl_initialized) {
        // set pixel format for OpenGL context
        int attrib[] =
        {
            WGL_DRAW_TO_WINDOW_ARB,
            true,
            WGL_SUPPORT_OPENGL_ARB,
            true,
            WGL_DOUBLE_BUFFER_ARB,
            true,
            WGL_PIXEL_TYPE_ARB,
            WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB,
            32,
            WGL_DEPTH_BITS_ARB,
            24,
            WGL_STENCIL_BITS_ARB,
            8,

        // uncomment for sRGB framebuffer, from WGL_ARB_framebuffer_sRGB extension
        // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
        // WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

        // uncomment for multisampeld framebuffer, from WGL_ARB_multisample extension
        // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
    #if MU_GL_ENABLE_MULTISAMPLING
            WGL_SAMPLE_BUFFERS_ARB,
            1,
            WGL_SAMPLES_ARB,
            4, // 4x MSAA
    #endif

            0,
        };

        int format;
        UINT formats;
        if (!wglChoosePixelFormatARB(w32_window->handle_dc, attrib, 0, 1, &format, &formats) || formats == 0) {
            MU_ASSERT(!"OpenGL does not support required pixel format!");
        }

        PIXELFORMATDESCRIPTOR desc;
        MU__ZeroMemory(&desc, sizeof(desc));
        desc.nSize = sizeof(desc);
        int ok = DescribePixelFormat(w32_window->handle_dc, format, sizeof(desc), &desc);
        MU_ASSERT(ok && "Failed to describe OpenGL pixel format");

        if (!SetPixelFormat(w32_window->handle_dc, format, &desc)) {
            MU_ASSERT(!"Cannot set OpenGL selected pixel format!");
        }

        // create modern OpenGL context
        {
            int attrib[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB,
                mu->opengl_major,
                WGL_CONTEXT_MINOR_VERSION_ARB,
                mu->opengl_minor,
                WGL_CONTEXT_PROFILE_MASK_ARB,
                WGL_CONTEXT_CORE_PROFILE_BIT_ARB,

    #if MU_GL_BUILD_DEBUG
                WGL_CONTEXT_FLAGS_ARB,
                WGL_CONTEXT_DEBUG_BIT_ARB,
    #endif

                0,
            };

            HGLRC rc = wglCreateContextAttribsARB(w32_window->handle_dc, 0, attrib);
            if (!rc) {
                MU_ASSERT(!"Cannot create modern OpenGL context! OpenGL version 4.5 not supported?");
            }

            BOOL ok = mu_wglMakeCurrent(w32_window->handle_dc, rc);
            MU_ASSERT(ok && "Failed to make current OpenGL context");
        }
    }
}

//
// Sound using WASAPI
// @! Sound: Comeback to it later! I dont really know what I should expect from a sound system
// What actually in reality errors out in WASAPI, what is important when working with sound.
// As such I'm not really currently equiped to make something good / reliable.
// Probably would be nice to work with it a bit more.
//
// Sound params should probably be configurable
// but I dont really understand what I should want to expect
// from this sort of system
//
// Not sure if I should in the future implement some different non threaded api.
//
//
// Below GUID stuff taken from libsoundio
// reference: https://github.com/andrewrk/libsoundio/blob/master/src/wasapi.c
//

// And some GUID are never implemented (Ignoring the INITGUID define)
MU_PRIVATE_VAR const CLSID MU_CLSID_MMDeviceEnumerator = {
    0xbcde0395, 0xe52f, 0x467c, {0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e}
};
MU_PRIVATE_VAR const IID MU_IID_IMMDeviceEnumerator = {
  //  MIDL_INTERFACE("A95664D2-9614-4F35-A746-DE8DB63617E6")
    0xa95664d2,
    0x9614,
    0x4f35,
    {0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6}
};
MU_PRIVATE_VAR const IID MU_IID_IMMNotificationClient = {
  //  MIDL_INTERFACE("7991EEC9-7E89-4D85-8390-6C703CEC60C0")
    0x7991eec9,
    0x7e89,
    0x4d85,
    {0x83, 0x90, 0x6c, 0x70, 0x3c, 0xec, 0x60, 0xc0}
};
MU_PRIVATE_VAR const IID MU_IID_IAudioClient = {
  //  MIDL_INTERFACE("1CB9AD4C-DBFA-4c32-B178-C2F568A703B2")
    0x1cb9ad4c,
    0xdbfa,
    0x4c32,
    {0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2}
};
MU_PRIVATE_VAR const IID MU_IID_IAudioRenderClient = {
  //  MIDL_INTERFACE("F294ACFC-3146-4483-A7BF-ADDCA7C260E2")
    0xf294acfc,
    0x3146,
    0x4483,
    {0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2}
};
MU_PRIVATE_VAR const IID MU_IID_IAudioSessionControl = {
  //  MIDL_INTERFACE("F4B1A599-7266-4319-A8CA-E70ACB11E8CD")
    0xf4b1a599,
    0x7266,
    0x4319,
    {0xa8, 0xca, 0xe7, 0x0a, 0xcb, 0x11, 0xe8, 0xcd}
};
MU_PRIVATE_VAR const IID MU_IID_IAudioSessionEvents = {
  //  MIDL_INTERFACE("24918ACC-64B3-37C1-8CA9-74A66E9957A8")
    0x24918acc,
    0x64b3,
    0x37c1,
    {0x8c, 0xa9, 0x74, 0xa6, 0x6e, 0x99, 0x57, 0xa8}
};
MU_PRIVATE_VAR const IID MU_IID_IMMEndpoint = {
  //  MIDL_INTERFACE("1BE09788-6894-4089-8586-9A2A6C265AC5")
    0x1be09788,
    0x6894,
    0x4089,
    {0x85, 0x86, 0x9a, 0x2a, 0x6c, 0x26, 0x5a, 0xc5}
};
MU_PRIVATE_VAR const IID MU_IID_IAudioClockAdjustment = {
  //  MIDL_INTERFACE("f6e4c0a0-46d9-4fb8-be21-57a3ef2b626c")
    0xf6e4c0a0,
    0x46d9,
    0x4fb8,
    {0xbe, 0x21, 0x57, 0xa3, 0xef, 0x2b, 0x62, 0x6c}
};
MU_PRIVATE_VAR const IID MU_IID_IAudioCaptureClient = {
  //  MIDL_INTERFACE("C8ADBD64-E71E-48a0-A4DE-185C395CD317")
    0xc8adbd64,
    0xe71e,
    0x48a0,
    {0xa4, 0xde, 0x18, 0x5c, 0x39, 0x5c, 0xd3, 0x17}
};
MU_PRIVATE_VAR const IID MU_IID_ISimpleAudioVolume = {
  //  MIDL_INTERFACE("87ce5498-68d6-44e5-9215-6da47ef883d8")
    0x87ce5498,
    0x68d6,
    0x44e5,
    {0x92, 0x15, 0x6d, 0xa4, 0x7e, 0xf8, 0x83, 0xd8}
};

    #ifdef __cplusplus
        // In C++ mode, IsEqualGUID() takes its arguments by reference
        #define IS_EQUAL_GUID(a, b) IsEqualGUID(*(a), *(b))
        #define IS_EQUAL_IID(a, b) IsEqualIID((a), *(b))

        // And some constants are passed by reference
        #define MU_IID_IAUDIOCLIENT (MU_IID_IAudioClient)
        #define MU_IID_IMMENDPOINT (MU_IID_IMMEndpoint)
        #define MU_IID_IAUDIOCLOCKADJUSTMENT (MU_IID_IAudioClockAdjustment)
        #define MU_IID_IAUDIOSESSIONCONTROL (MU_IID_IAudioSessionControl)
        #define MU_IID_IAUDIORENDERCLIENT (MU_IID_IAudioRenderClient)
        #define MU_IID_IMMDEVICEENUMERATOR (MU_IID_IMMDeviceEnumerator)
        #define MU_IID_IAUDIOCAPTURECLIENT (MU_IID_IAudioCaptureClient)
        #define MU_IID_ISIMPLEAUDIOVOLUME (MU_IID_ISimpleAudioVolume)
        #define MU_CLSID_MMDEVICEENUMERATOR (MU_CLSID_MMDeviceEnumerator)
        #define MU_PKEY_DEVICE_FRIENDLYNAME (PKEY_Device_FriendlyName)
        #define MU_PKEY_AUDIOENGINE_DEVICEFORMAT (PKEY_AudioEngine_DeviceFormat)

    #else
        #define IS_EQUAL_GUID(a, b) IsEqualGUID((a), (b))
        #define IS_EQUAL_IID(a, b) IsEqualIID((a), (b))

        #define MU_IID_IAUDIOCLIENT (&MU_IID_IAudioClient)
        #define MU_IID_IMMENDPOINT (&MU_IID_IMMEndpoint)
        #define MU_PKEY_DEVICE_FRIENDLYNAME (&PKEY_Device_FriendlyName)
        #define MU_PKEY_AUDIOENGINE_DEVICEFORMAT (&PKEY_AudioEngine_DeviceFormat)
        #define MU_CLSID_MMDEVICEENUMERATOR (&MU_CLSID_MMDeviceEnumerator)
        #define MU_IID_IAUDIOCLOCKADJUSTMENT (&MU_IID_IAudioClockAdjustment)
        #define MU_IID_IAUDIOSESSIONCONTROL (&MU_IID_IAudioSessionControl)
        #define MU_IID_IAUDIORENDERCLIENT (&MU_IID_IAudioRenderClient)
        #define MU_IID_IMMDEVICEENUMERATOR (&MU_IID_IMMDeviceEnumerator)
        #define MU_IID_IAUDIOCAPTURECLIENT (&MU_IID_IAudioCaptureClient)
        #define MU_IID_ISIMPLEAUDIOVOLUME (&MU_IID_ISimpleAudioVolume)
    #endif

    // Number of REFERENCE_TIME units per second
    // One unit is equal to 100 nano seconds
    #define MU_REF_TIMES_PER_SECOND 10000000
    #define MU_REF_TIMES_PER_MSECOND 10000

// Empty functions(stubs) which are used when library fails to load
static HRESULT CoCreateInstanceStub(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv) {
    (void)(rclsid);
    (void)(pUnkOuter);
    (void)(dwClsContext);
    (void)(riid);
    (void)(ppv);
    return S_FALSE;
}

static HRESULT CoInitializeExStub(LPVOID pvReserved, DWORD dwCoInit) {
    (void)(pvReserved);
    (void)(dwCoInit);
    return S_FALSE;
}

MU_StaticFunc void MU_WIN32_DeinitSound(MU_Context *mu) {
    MU_Win32 *w32 = (MU_Win32 *)mu->platform;
    if (w32->audio_client) w32->audio_client->lpVtbl->Stop(w32->audio_client);
    if (w32->audio_client) w32->audio_client->lpVtbl->Release(w32->audio_client);
    if (w32->device_enum) w32->device_enum->lpVtbl->Release(w32->device_enum);
    if (w32->device) w32->device->lpVtbl->Release(w32->device);
    if (w32->audio_render_client) w32->audio_render_client->lpVtbl->Release(w32->audio_render_client);
    mu->sound.initialized = false;
}

// Load COM Library functions dynamically,
// this way sound is not necessary to run the game
MU_StaticFunc void MU_WIN32_LoadCOM(MU_Context *mu) {
    MU_Win32 *w32 = (MU_Win32 *)mu->platform;

    HMODULE ole32_lib = LoadLibraryA("ole32.dll");
    if (ole32_lib) {
        w32->CoCreateInstanceFunctionPointer = (CoCreateInstanceFunction *)GetProcAddress(ole32_lib, "CoCreateInstance");
        w32->CoInitializeExFunctionPointer = (CoInitializeExFunction *)GetProcAddress(ole32_lib, "CoInitializeEx");
        mu->sound.initialized = true;
    }

    if (ole32_lib == 0 || w32->CoCreateInstanceFunctionPointer == 0 || w32->CoInitializeExFunctionPointer == 0) {
        w32->CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
        w32->CoInitializeExFunctionPointer = CoInitializeExStub;
        mu->sound.initialized = false;
    }
}

MU_StaticFunc DWORD MU_WIN32_SoundThread(void *parameter) {
    MU_Context *mu = (MU_Context *)parameter;
    MU_Win32 *w32 = (MU_Win32 *)mu->platform;

    HANDLE thread_handle = GetCurrentThread();
    SetThreadPriority(thread_handle, THREAD_PRIORITY_HIGHEST);
    HANDLE buffer_ready_event = CreateEvent(0, 0, 0, 0);
    if (!buffer_ready_event) {
        MU_ASSERT(!"Sound thread failed");
        goto error_cleanup;
    }
    if (FAILED(IAudioClient_SetEventHandle(w32->audio_client, buffer_ready_event))) {
        MU_ASSERT(!"Sound thread failed");
        goto error_cleanup;
    }

    if (FAILED(IAudioClient_Start(w32->audio_client))) {
        MU_ASSERT(!"Sound thread failed");
        goto error_cleanup;
    }
    for (;;) {
        if (WaitForSingleObject(buffer_ready_event, INFINITE) != WAIT_OBJECT_0) {
            MU_ASSERT(!"Sound thread failed");
            goto error_cleanup;
        }
        uint32_t padding_frame_count;
        if (FAILED(IAudioClient_GetCurrentPadding(w32->audio_client, &padding_frame_count))) {
            MU_ASSERT(!"Sound thread failed");
            goto error_cleanup;
        }
        uint32_t *samples;
        uint32_t fill_frame_count = w32->buffer_frame_count - padding_frame_count;
        if (FAILED(IAudioRenderClient_GetBuffer(w32->audio_render_client, fill_frame_count, (BYTE **)&samples))) {
            MU_ASSERT(!"Sound thread failed");
            goto error_cleanup;
        }

        // Call user callback
        uint32_t sample_count_to_fill = fill_frame_count * mu->sound.number_of_channels;
        mu->sound.callback((MU_Context *)mu, (uint16_t *)samples, sample_count_to_fill);

        if (FAILED(IAudioRenderClient_ReleaseBuffer(w32->audio_render_client, fill_frame_count, 0))) {
            MU_ASSERT(!"Sound thread failed");
            goto error_cleanup;
        }
    }
    return 0;
error_cleanup:
    MU_WIN32_DeinitSound(mu);
    return -1;
}

MU_StaticFunc void MU_WIN32_InitWasapi(MU_Context *mu) {
    REFERENCE_TIME requested_buffer_duration = MU_REF_TIMES_PER_MSECOND * 40;
    MU_Win32 *w32 = (MU_Win32 *)mu->platform;

    MU_WIN32_LoadCOM(mu);
    MU_ASSERT(mu->sound.initialized);
    if (mu->sound.initialized == false) {
        return;
    }

    mu->sound.bytes_per_sample = 2;
    mu->sound.number_of_channels = 2;
    mu->sound.samples_per_second = 44100;

    HANDLE thread_handle;

    HRESULT hr = w32->CoInitializeExFunctionPointer(0, COINITBASE_MULTITHREADED);
    if (FAILED(hr)) {
        MU_ASSERT(!"Failed to initialize sound");
        goto failure_path;
    }

    hr = w32->CoCreateInstanceFunctionPointer(MU_CLSID_MMDEVICEENUMERATOR, NULL, CLSCTX_ALL, MU_IID_IMMDEVICEENUMERATOR, (void **)&w32->device_enum);
    if (FAILED(hr)) {
        MU_ASSERT(!"Failed to initialize sound");
        goto failure_path;
    }

    hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(w32->device_enum, eRender, eMultimedia, &w32->device);
    if (FAILED(hr)) {
        MU_ASSERT(!"Failed to initialize sound");
        goto failure_path;
    }

    hr = IMMDevice_Activate(w32->device, MU_IID_IAUDIOCLIENT, CLSCTX_ALL, NULL, (void **)&w32->audio_client);
    if (FAILED(hr)) {
        MU_ASSERT(!"Failed to initialize sound");
        goto failure_path;
    }

    WAVEFORMATEX fmt;
    {
        MU__ZeroMemory(&fmt, sizeof(fmt));
        fmt.wFormatTag = WAVE_FORMAT_PCM;
        fmt.nChannels = mu->sound.number_of_channels;
        fmt.nSamplesPerSec = mu->sound.samples_per_second;
        fmt.wBitsPerSample = mu->sound.bytes_per_sample * 8;
        fmt.nBlockAlign = fmt.nChannels * fmt.wBitsPerSample / 8;
        fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
    }

    hr = IAudioClient_Initialize(
        w32->audio_client, AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_RATEADJUST |
            AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
        requested_buffer_duration, 0, &fmt, 0);
    if (FAILED(hr)) {
        MU_ASSERT(!"Failed to initialize sound");
        goto failure_path;
    }

    hr = IAudioClient_GetService(w32->audio_client, MU_IID_IAUDIORENDERCLIENT, (void **)&w32->audio_render_client);
    if (FAILED(hr)) {
        MU_ASSERT(!"Failed to initialize sound");
        goto failure_path;
    }

    hr = IAudioClient_GetBufferSize(w32->audio_client, &w32->buffer_frame_count);
    if (FAILED(hr)) {
        MU_ASSERT(!"Failed to initialize sound");
        goto failure_path;
    }

    thread_handle = CreateThread(0, 0, MU_WIN32_SoundThread, mu, 0, 0);
    if (thread_handle == INVALID_HANDLE_VALUE) {
        MU_ASSERT(!"Failed to create a sound thread");
        goto failure_path;
    }

    return;
failure_path:
    MU_WIN32_DeinitSound(mu);
}

#endif // _WIN32