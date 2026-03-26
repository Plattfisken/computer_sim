#include "ui_layout.h"
#include "computer_sim.h"

#define CLAY_IMPLEMENTATION
#include "../third_party/clay.h"


#include <assert.h>

#define FONT_SIZE 16
#define UI_OUTER_BACKGROUND_COLOR (Clay_Color){ 100, 100, 100, 255 }
#define UI_INNER_BACKGROUND_COLOR (Clay_Color){ 170, 170, 170, 255 }
#define BUTTON_COLOR (Clay_Color){ 100, 100, 100, 255 }
#define BUTTON_HOVERED_COLOR (Clay_Color){ 100, 100, 200, 255 }
#define ROWS_PER_PAGE 16
#define COLS_PER_PAGE 8


struct UiLayoutState {
    Clay_Context *clay_context;
    Font font;
    int current_memory_page;
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

#define draw_text_default(str, fs) draw_text_default_(CLAY_STRING(str), fs)
static void draw_text_default_(Clay_String s, int font_size) {
    CLAY_TEXT(s, CLAY_TEXT_CONFIG({
        .fontId = 0,
        .fontSize = font_size,
        .textColor = { 0, 0, 0, 255 },
    }));
}

#define draw_button(label) draw_button_(CLAY_STRING(label))
static bool draw_button_(Clay_String label) {
    bool result = false;
    CLAY(CLAY_SID(label), { .backgroundColor = Clay_Hovered() ? BUTTON_HOVERED_COLOR : BUTTON_COLOR }) {
        draw_text_default_(label, FONT_SIZE);
        if(Clay_Hovered() && Clay_GetCurrentContext()->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
            result = true;
        }
    }
    return result;
}

static void draw_uint8(uint8_t num, UT_Arena *arena, UT_String append) {
    int num_chars = 4;
    // + 1 because of snprintf forces last character to be null
    char *buf = UT_arena_alloc(arena, num_chars + append.length + 1, 1);
    snprintf(buf, num_chars + append.length + 1, "0x%02x%.*s", num, (int)append.length, append.data);
    Clay_String num_string = { .length = num_chars + append.length, .chars = buf };
    draw_text_default_(num_string, FONT_SIZE);
}

static void draw_uint16(uint16_t num, UT_Arena *arena, UT_String append) {
    int num_chars = 6;
    // + 1 because of snprintf forces last character to be null
    char *buf = UT_arena_alloc(arena, num_chars + append.length + 1, 1);
    snprintf(buf, num_chars + append.length + 1, "0x%04x%.*s", num, (int)append.length, append.data);
    Clay_String num_string = { .length = num_chars + append.length, .chars = buf };
    draw_text_default_(num_string, FONT_SIZE);
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
                    .width = CLAY_SIZING_PERCENT(0.5),
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
            draw_text_default("Registers: ", 20);
            for(uint32_t i = 0; i < UT_ARRAY_LENGTH(app_state->computer.registers); ++i) {
                draw_uint16(app_state->computer.registers[i], frame_arena, UT_STR(""));
            }
            draw_text_default("Program Counter: ", 20);
            draw_uint16(app_state->computer.program_counter, frame_arena, UT_STR(""));
            draw_text_default("Memory: ", 20);
            uint16_t address = ROWS_PER_PAGE * COLS_PER_PAGE * my_state->current_memory_page;
            for(uint32_t row = 0; row < ROWS_PER_PAGE; ++row) {
                CLAY(CLAY_IDI("Memory row", row), { .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 4 }}) {
                    draw_uint16(address, frame_arena, UT_STR(":"));
                    for(uint32_t col = 0; col < COLS_PER_PAGE; ++col) {
                        uint8_t byte_value = *(uint8_t *)computer_pointer_from_virtual_address(&app_state->computer, address++);
                        draw_uint8(byte_value, frame_arena, UT_STR(""));
                    }
                }
            }
            CLAY(CLAY_ID("Button row"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW() }, .layoutDirection = CLAY_LEFT_TO_RIGHT }}) {
                if(draw_button("Prev page")) {
                    if(my_state->current_memory_page > 0)
                        --my_state->current_memory_page;
                }
                CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() }}}) {}
                if(draw_button("Next page")) {
                    //if(my_state->current_memory_page < a)
                    ++my_state->current_memory_page;
                }
            }
        }
    }
    Clay_RenderCommandArray render_commands = Clay_EndLayout();
    Clay_Raylib_Render(render_commands, &my_state->font, frame_arena);
    UT_arena_free(frame_arena);
}
// TODO: is this really the way to do this..?
#include "computer_sim.c"
// Implementations in this translation unit
#define USEFUL_THINGS_IMPLEMENTATION
#include "../third_party/useful_things.h"
