#include "ui_layout.h"

#define CLAY_IMPLEMENTATION
#include "../third_party/clay.h"


#include <assert.h>

#define FONT_SIZE 16
#define UI_OUTER_BACKGROUND_COLOR (Clay_Color){ 100, 100, 100, 255 }
#define UI_INNER_BACKGROUND_COLOR (Clay_Color){ 170, 170, 170, 255 }
//#define UI_INNER_BACKGROUND_COLOR (Clay_Color){ 255, 0, 0, 255 }

struct UiLayoutState {
    Clay_Context *clay_context;
    //Clay_Arena clay_arena;
    Font font;
};

#include "../third_party/clay_renderer_raylib.c"

void clay_handle_error_callback(Clay_ErrorData error_data) {
    printf("clay error: %.*s\n", error_data.errorText.length, error_data.errorText.chars);
}

static Clay_Dimensions ui_get_clay_layout_dimensions(void) {
    return (Clay_Dimensions){ (float)GetScreenWidth(), (float)GetScreenHeight() };
}

// NOTE: the UILayoutState pointer returned by this function should be kept by the caller for the entire lifetime of the UI (probably entire lifetime of the app), it should persist even between reloads of the ui shared object. On reload theres no need to initialize again. Just make sure the state is kept and passed to calls to render_ui_layout, and that the arena lives on also for the entire lifetime of the UI
UiLayoutState *init_ui_engine(UT_Arena *arena, Font font) {
    UiLayoutState *state = UT_arena_alloc(arena, sizeof *state, sizeof (size_t));
    memset(state, 0, sizeof *state);

    uint64_t required_memory = Clay_MinMemorySize();
    void *memory = UT_arena_alloc(arena, required_memory, sizeof (size_t));
    assert(memory && "Failed to allocate");
    Clay_Arena clay_arena = Clay_CreateArenaWithCapacityAndMemory(required_memory, memory);
    state->clay_context = Clay_Initialize(clay_arena, ui_get_clay_layout_dimensions(), (Clay_ErrorHandler){ clay_handle_error_callback, NULL });

    state->font = font;
    SetTextureFilter(state->font.texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, &state->font);
    return state;
}

// to be called after every reload, most state should survive fine after reload so there's no need to call init function again.
// But at least one thing has to be done, reset the error handle callback, since the old address of the function pointer is invalid.
// Clay doesn't offer any handy setter for this so we have to do it manually, thankfully the struct is fully transparent
void reload_ui_engine(UiLayoutState *state) {
    state->clay_context->errorHandler = (Clay_ErrorHandler){ clay_handle_error_callback, NULL };
    Clay_SetCurrentContext(state->clay_context);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, &state->font);
    Clay_SetDebugModeEnabled(true);
}

// NOTE: this function should be called inbetween calls to raylibs BeginDrawing() and EndDrawing()
void render_ui_layout(AppState *app_state, UiLayoutState *my_state) {
    UT_Arena *frame_arena = UT_arena_create();

    Clay_SetCurrentContext(my_state->clay_context);
    Clay_SetLayoutDimensions(ui_get_clay_layout_dimensions());
    Vector2 mouse_pos = GetMousePosition();
    Clay_SetPointerState((Clay_Vector2){ .x = mouse_pos.x, .y = mouse_pos.y }, IsMouseButtonDown(MOUSE_BUTTON_LEFT));
    Vector2 raylib_scroll_delta = GetMouseWheelMoveV();
    Clay_Vector2 scroll_delta = { .x = raylib_scroll_delta.x, .y = raylib_scroll_delta.y };
    Clay_UpdateScrollContainers(false, scroll_delta, GetFrameTime());
    Clay_BeginLayout();

    int gap = 8;
    CLAY(CLAY_ID("Outer container"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .padding = { gap, gap, gap, gap },
            .childGap = gap,
        },
        .backgroundColor = UI_OUTER_BACKGROUND_COLOR,
    }) {
        CLAY(CLAY_ID("Computer view"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.66),
                    //.width = CLAY_SIZING_PERCENT(0),
                    .height = CLAY_SIZING_GROW(0),
                }
            },
            .backgroundColor = UI_INNER_BACKGROUND_COLOR,
            .cornerRadius = CLAY_CORNER_RADIUS(8),
        }) {}
        CLAY(CLAY_ID("Editor view"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .padding = { gap, gap, gap, gap },
            },
            .backgroundColor = UI_INNER_BACKGROUND_COLOR,
            .cornerRadius = CLAY_CORNER_RADIUS(8),
        }) {
                CLAY_TEXT(CLAY_STRING("Registers: "), CLAY_TEXT_CONFIG({
                    .fontId = 0,
                    .fontSize = 22,
                    .textColor = { 0, 0, 0, 255 },
                }));
                for(uint32_t i = 0; i < UT_ARRAY_LENGTH(app_state->computer.registers); ++i) {
                    char *buf = UT_arena_alloc(frame_arena, 11, 1);
                    snprintf(buf, 11, "0x%08x", app_state->computer.registers[i]);
                    Clay_String register_string = { .length = 10, .chars = buf };
                    CLAY_TEXT(register_string, CLAY_TEXT_CONFIG({
                                .fontId = 0,
                                .fontSize = FONT_SIZE,
                                .textColor = { 0, 0, 0, 255 },
                                }));
                }
        }
    }
    Clay_RenderCommandArray render_commands = Clay_EndLayout();
    Clay_Raylib_Render(render_commands, &my_state->font, frame_arena);
    UT_arena_free(frame_arena);
}
// Implementations in this translation unit
#define USEFUL_THINGS_IMPLEMENTATION
#include "../third_party/useful_things.h"
