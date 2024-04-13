#ifndef FIRST_MU_HEADER
#define FIRST_MU_HEADER
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#ifndef MU_API
    #ifdef __cplusplus
        #define MU_API extern "C"
    #else
        #define MU_API
    #endif
#endif

#ifndef MU_INLINE
    #ifndef _MSC_VER
        #ifdef __cplusplus
            #define MU_INLINE inline
        #else
            #define MU_INLINE
        #endif
    #else
        #define MU_INLINE __forceinline
    #endif
#endif

#ifndef MU_Float2
    #define MU_Float2 MU__Float2
typedef struct MU__Float2 {
    float x;
    float y;
} MU__Float2;
#endif

#ifndef MU_Int2
    #define MU_Int2 MU__Int2
typedef struct MU__Int2 {
    int x;
    int y;
} MU__Int2;
#endif

//@begin gen_structs
typedef struct MU_UTF32Result MU_UTF32Result;
typedef struct MU_UTF8Result MU_UTF8Result;
typedef struct MU_Win32 MU_Win32;
typedef struct MU_Win32_Window MU_Win32_Window;
typedef struct MU_Window_Params MU_Window_Params;
typedef struct MU_Params MU_Params;
typedef struct MU_Key_State MU_Key_State;
typedef struct MU_Mouse_State MU_Mouse_State;
typedef struct MU_DroppedFile MU_DroppedFile;
typedef struct MU_Arena MU_Arena;
typedef struct MU_Window MU_Window;
typedef struct MU_Time MU_Time;
typedef struct MU_Sound MU_Sound;
typedef struct MU_Context MU_Context;
//@end gen_structs

typedef void *MU_glGetProcAddress(const char *);

struct MU_Window_Params {
    MU_Int2 size;
    MU_Int2 pos;
    char *title;
    bool enable_canvas;
    bool resizable;
    bool borderless;
    bool fps_cursor;
};

struct MU_Params {
    void *memory;
    size_t cap;

    bool enable_opengl;
    int opengl_major;
    int opengl_minor;

    double delta_time;
    MU_Window_Params window; // this controls window when calling MU_Start
    void (*sound_callback)(MU_Context *mu, uint16_t *buffer, uint32_t samples_to_fill);
};

struct MU_Key_State {
    bool down;
    bool press;
    bool unpress;
    bool raw_press;
};

typedef enum MU_Key {
    MU_KEY_INVALID,
    MU_KEY_ESCAPE,
    MU_KEY_ENTER,
    MU_KEY_TAB,
    MU_KEY_BACKSPACE,
    MU_KEY_INSERT,
    MU_KEY_DELETE,
    MU_KEY_RIGHT,
    MU_KEY_LEFT,
    MU_KEY_DOWN,
    MU_KEY_UP,
    MU_KEY_PAGE_UP,
    MU_KEY_PAGE_DOWN,
    MU_KEY_HOME,
    MU_KEY_END,
    MU_KEY_F1,
    MU_KEY_F2,
    MU_KEY_F3,
    MU_KEY_F4,
    MU_KEY_F5,
    MU_KEY_F6,
    MU_KEY_F7,
    MU_KEY_F8,
    MU_KEY_F9,
    MU_KEY_F10,
    MU_KEY_F11,
    MU_KEY_F12,
    MU_KEY_SPACE = 32,
    MU_KEY_APOSTROPHE = 39,
    MU_KEY_PLUS = 43,
    MU_KEY_COMMA = 44,
    MU_KEY_MINUS = 45,
    MU_KEY_PERIOD = 46,
    MU_KEY_SLASH = 47,
    MU_KEY_0 = 48,
    MU_KEY_1 = 49,
    MU_KEY_2 = 50,
    MU_KEY_3 = 51,
    MU_KEY_4 = 52,
    MU_KEY_5 = 53,
    MU_KEY_6 = 54,
    MU_KEY_7 = 55,
    MU_KEY_8 = 56,
    MU_KEY_9 = 57,
    MU_KEY_SEMICOLON = 59,
    MU_KEY_EQUAL = 61,
    MU_KEY_A = 65,
    MU_KEY_B = 66,
    MU_KEY_C = 67,
    MU_KEY_D = 68,
    MU_KEY_E = 69,
    MU_KEY_F = 70,
    MU_KEY_G = 71,
    MU_KEY_H = 72,
    MU_KEY_I = 73,
    MU_KEY_J = 74,
    MU_KEY_K = 75,
    MU_KEY_L = 76,
    MU_KEY_M = 77,
    MU_KEY_N = 78,
    MU_KEY_O = 79,
    MU_KEY_P = 80,
    MU_KEY_Q = 81,
    MU_KEY_R = 82,
    MU_KEY_S = 83,
    MU_KEY_T = 84,
    MU_KEY_U = 85,
    MU_KEY_V = 86,
    MU_KEY_W = 87,
    MU_KEY_X = 88,
    MU_KEY_Y = 89,
    MU_KEY_Z = 90,
    MU_KEY_LEFT_BRACKET = 91,
    MU_KEY_BACKSLASH = 92,
    MU_KEY_RIGHT_BRACKET = 93,
    MU_KEY_GRAVE_ACCENT = 96,
    MU_KEY_F13,
    MU_KEY_F14,
    MU_KEY_F15,
    MU_KEY_F16,
    MU_KEY_F17,
    MU_KEY_F18,
    MU_KEY_F19,
    MU_KEY_F20,
    MU_KEY_F21,
    MU_KEY_F22,
    MU_KEY_F23,
    MU_KEY_F24,
    MU_KEY_KP_0,
    MU_KEY_KP_1,
    MU_KEY_KP_2,
    MU_KEY_KP_3,
    MU_KEY_KP_4,
    MU_KEY_KP_5,
    MU_KEY_KP_6,
    MU_KEY_KP_7,
    MU_KEY_KP_8,
    MU_KEY_KP_9,
    MU_KEY_KP_DECIMAL,
    MU_KEY_KP_DIVIDE,
    MU_KEY_KP_MULTIPLY,
    MU_KEY_KP_SUBTRACT,
    MU_KEY_KP_ADD,
    MU_KEY_KP_ENTER,
    MU_KEY_LEFT_SHIFT,
    MU_KEY_LEFT_CONTROL,
    MU_KEY_LEFT_ALT,
    MU_KEY_LEFT_SUPER,
    MU_KEY_RIGHT_SHIFT,
    MU_KEY_RIGHT_CONTROL,
    MU_KEY_RIGHT_ALT,
    MU_KEY_RIGHT_SUPER,
    MU_KEY_CAPS_LOCK,
    MU_KEY_SCROLL_LOCK,
    MU_KEY_NUM_LOCK,
    MU_KEY_PRINT_SCREEN,
    MU_KEY_PAUSE,
    MU_KEY_SHIFT,
    MU_KEY_CONTROL,
    MU_KEY_COUNT,
} MU_Key;

struct MU_Mouse_State {
    MU_Int2 pos;
    MU_Float2 posf;
    MU_Int2 delta_pos;
    MU_Float2 delta_pos_normalized;
    MU_Key_State left;
    MU_Key_State middle;
    MU_Key_State right;
    float delta_wheel; // @todo: add smooth delta?
};

struct MU_DroppedFile {
    MU_DroppedFile *next;
    char *filename; // null terminated
    int filename_size;
};

struct MU_Arena {
    char *memory;
    size_t len;
    size_t cap;
};

// Most of the fields in the window struct are read only. They are updated
// in appropriate update functions. The window should belong to the MU_Context
// but you get access to the information.
struct MU_Window {
    MU_Int2 size;
    MU_Float2 sizef;
    MU_Int2 pos;
    MU_Float2 posf;
    float dpi_scale;
    bool is_fullscreen;
    bool is_fps_mode;
    bool is_focused;
    bool change_cursor_on_mouse_hold; // @in @out
    uint64_t processed_events_this_frame;
    bool should_render; // @in @out this is false on first frame but it doesn't matter cause it shouldnt be rendered

    MU_DroppedFile *first_dropped_file;

    uint32_t *canvas;
    bool canvas_enabled; // @in @out

    MU_Mouse_State mouse;
    MU_Key_State key[MU_KEY_COUNT];

    uint32_t user_text32[32];
    int user_text32_count;

    char user_text8[32];
    int user_text8_count;

    MU_Window *next;
    void *handle;
    void *platform;
};

struct MU_Time {
    double app_start;
    double frame_start;

    double update;
    double update_total;

    double delta;
    float deltaf;
    double total;
    float totalf;
};

struct MU_Sound {
    bool initialized;
    unsigned samples_per_second;
    unsigned number_of_channels;
    unsigned bytes_per_sample;
    void (*callback)(MU_Context *mu, uint16_t *buffer, uint32_t samples_to_fill);
};

struct MU_Context {
    bool quit;

    MU_Sound sound;
    MU_Time time;
    bool first_frame;
    int _MU_Update_count;
    size_t frame;
    size_t consecutive_missed_frames;
    size_t total_missed_frames;

    MU_Int2 primary_monitor_size;
    bool opengl_initialized;
    int opengl_major;
    int opengl_minor;
    void *(*gl_get_proc_address)(const char *str);

    MU_Params params;
    MU_Window *window;
    MU_Window *all_windows;
    MU_Arena perm_arena;
    MU_Arena frame_arena; // Reset at beginning of MU_Update
    void *platform;
};

//@begin gen_api_funcs
MU_API void MU_Quit(MU_Context *mu);
MU_API void MU_DefaultSoundCallback(MU_Context *mu, uint16_t *buffer, uint32_t samples_to_fill);
MU_API double MU_GetTime(void);
MU_API void MU_ToggleFPSMode(MU_Window *window);
MU_API void MU_DisableFPSMode(MU_Window *window);
MU_API void MU_EnableFPSMode(MU_Window *window);
MU_API void MU_ToggleFullscreen(MU_Window *window);
MU_API void MU_Init(MU_Context *mu, MU_Params params, size_t len);
MU_API MU_Window *MU_AddWindow(MU_Context *mu, MU_Window_Params params);
MU_API void MU_InitWindow(MU_Context *mu, MU_Window *window, MU_Window_Params params);
MU_API MU_Context *MU_Start(MU_Params params);
MU_API bool MU_Update(MU_Context *mu);
//@end gen_api_funcs

/* @! In the future, api for processing messages manually

    while(true) {
        MU_Event event;
        while (mu_get_event_blocking(&event)) {
            switch(event.kind) {

            }
        }
    }

typedef int MU_Modifier;
enum MU_Modifier {
    MU_MODIFIER_SHIFT = 0x1,      // left or right shift key
    MU_MODIFIER_CTRL  = 0x2,      // left or right control key
    MU_MODIFIER_ALT   = 0x4,      // left or right alt key
    MU_MODIFIER_SUPER = 0x8,      // left or right 'super' key
    MU_MODIFIER_LMB   = 0x100,    // left mouse button
    MU_MODIFIER_RMB   = 0x200,    // right mouse button
    MU_MODIFIER_MMB   = 0x400,    // middle mouse button
};

typedef enum MU_Event_Kind {
    MU_EVENT_KIND_INVALID,
    MU_EVENT_KIND_KEY_DOWN,
    MU_EVENT_KIND_KEY_UP,
    MU_EVENT_KIND_MOUSE_MOVE,
} MU_Event_Kind;

typedef struct MU_Event {
    MU_Event_Kind kind;
    MU_Modifier modifier;
    MU_Key key;
} MU_Event;


    */
#endif