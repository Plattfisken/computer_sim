#include <raylib.h>

#include "../third_party/useful_things.h"

#define NOB_IMPLEMENTATION
#define NOB_UNSTRIP_PREFIX
#include "../third_party/nob.h"

#include "app.h"
#include "computer_sim.h"
#include "text_editor.h"
#include "ui_layout.h"

#include <dlfcn.h>
#include <sys/stat.h>

#define BUTTON_WIDTH 150
#define BUTTON_HEIGHT 40
#define BUTTON_SPACING 10

//Color button_color(bool button_activated) {
//    return button_activated ? GREEN : RED;
//}
//
//void raylib_draw_uint16_hex(uint16_t num, int x, int y, String append) {
//    int buf_size = 16 + append.length;
//    char *buf = malloc(buf_size);
//    snprintf(buf, buf_size, "0x%04X %s", num, append.data);
//    DrawText(buf, x, y, FONT_SIZE, BLACK);
//    free(buf);
//}
//
//void raylib_draw_uint8_hex(uint8_t num, int x, int y, Color color,  String append) {
//    int buf_size = 16 + append.length;
//    char *buf = malloc(buf_size);
//    snprintf(buf, buf_size, "0x%02X %s", num, append.data);
//    DrawText(buf, x, y, FONT_SIZE, color);
//    free(buf);
//}
//
//Rectangle rectangle_centered(int width, int height) {
//    return (Rectangle){ GetScreenWidth()/2 - width/2, GetScreenHeight()/2 - height/2, width, height };
//}

static const char *ui_layout_lib_path = "../build/ui_layout.so";
static void *ui_layout_lib;
static UiLayoutState *ui_layout_state;
static init_ui_engine_func *init_ui_engine;
static reload_ui_engine_func *reload_ui_engine;
static render_ui_layout_func *render_ui_layout;

void reload_ui_layout() {
    bool initial_load = (ui_layout_lib == NULL);
    if(!initial_load) {
        printf("Lib already loaded, closing...\n");
        int ok = dlclose(ui_layout_lib);
        if(ok != 0) fprintf(stderr, "Failed to close lib: %s\n", dlerror());
        else printf("Library closed successfully, attempting reload...\n");
    } else {
        printf("Library not loaded, attempting first load...\n");
    }
    assert(dlopen(ui_layout_lib_path, RTLD_NOLOAD) == NULL && "Why?");
    ui_layout_lib = dlopen(ui_layout_lib_path, RTLD_NOW);
    if(!ui_layout_lib) {
        fprintf(stderr, "Failed to load library: %s\n", dlerror());
        return;
    }
    printf("Successfully loaded lib: %p\n", ui_layout_lib);

    init_ui_engine = dlsym(ui_layout_lib, "init_ui_engine");
    if(!init_ui_engine) {
        fprintf(stderr, "Failed to load symbol: %s\n", dlerror());
        return;
    }

    reload_ui_engine = dlsym(ui_layout_lib, "reload_ui_engine");
    if(!reload_ui_engine) {
        fprintf(stderr, "Failed to load symbol: %s\n", dlerror());
        return;
    }

    render_ui_layout = dlsym(ui_layout_lib, "render_ui_layout");
    if(!render_ui_layout) {
        fprintf(stderr, "Failed to load symbol: %s\n", dlerror());
        return;
    }
    if(!initial_load) {
        assert(ui_layout_state);
        reload_ui_engine(ui_layout_state);
    }
}

time_t last_file_modification_time(const char *path) {
    struct stat stat_result = {};
    if(stat(path, &stat_result) != 0) {
        // TODO: handle this
        printf("failed to stat\n");
        return 1;
    }
    return stat_result.st_mtimespec.tv_nsec;
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(16*80, 9*80, "Program");
    SetTargetFPS(60);


    AppState state = {};
    state.show_editor = true;
    computer_init(&state.computer);


    reload_ui_layout();
    UT_Arena *permanent_arena = UT_arena_create_size(MEGABYTES(64));
    Font font = LoadFont("../resources/Hack-Regular.ttf");
    ui_layout_state = init_ui_engine(permanent_arena, font);

    Image screen_img = {
        .data = computer_pointer_from_virtual_address(&state.computer, COMPUTER_SCREEN_BUFFER_DATA),
        .width = COMPUTER_SCREEN_BUFFER_WIDTH,
        .height = COMPUTER_SCREEN_BUFFER_HEIGHT,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        .mipmaps = 1
    };
    Texture2D texture = LoadTextureFromImage(screen_img);

    //TextBuffer text_buffer = load_file_into_text_buffer(STR("../test.asm"));

    const char *ui_layout_lib_src_path = "../src/ui_layout.c";
    time_t last_lib_src_modification_time = last_file_modification_time(ui_layout_lib_src_path);
    while(!WindowShouldClose()) {
        time_t lib_src_modification_time = last_file_modification_time(ui_layout_lib_src_path);
        if(lib_src_modification_time != last_lib_src_modification_time) {
            Nob_Cmd cmd = {};

            //nob_cmd_append(&cmd, "export", "ASAN_OPTIONS=detect_odr_violation=0");
            //if(!nob_cmd_run(&cmd)) {
            //    printf("what?");
            //    return 1;
            //}
            //setenv("ASAN_OPTIONS", "detect_odr_violation=0", true);
            nob_cmd_append(&cmd, "clang", "ui_layout.c", "-Wall", "-Wextra", "-Wno-unused-function", "-Wno-unused-parameter", "-g3", "-O0", "-I/opt/homebrew/Cellar/raylib/5.5/include", "-L/opt/homebrew/Cellar/raylib/5.5/lib", "-lraylib", "--shared", "-fPIC", "-o", "../build/ui_layout.so");
            if(nob_cmd_run(&cmd)) {
                reload_ui_layout();
            } else {
                printf("failed to recompile\n");
            }
            last_lib_src_modification_time = lib_src_modification_time;
        }

        if(IsKeyDown(KEY_A)) state.left = true;
        else state.left = false;
        if(IsKeyDown(KEY_D)) state.right = true;
        else state.right = false;
        if(IsKeyDown(KEY_W)) state.up = true;
        else state.up = false;
        if(IsKeyDown(KEY_S)) state.down = true;
        else state.down = false;


        BeginDrawing();
        ClearBackground(WHITE);
        UpdateTexture(texture, computer_pointer_from_virtual_address(&state.computer, COMPUTER_SCREEN_BUFFER_DATA));
        //Vector2 texture_pos = { 30.0, 30.0 };
        //DrawTextureEx(texture, texture_pos, 0, 8, WHITE);

        //DrawCircle(30, GetScreenHeight()-60, 10, button_color(state.left));
        //DrawCircle(60, GetScreenHeight()-90, 10, button_color(state.up));
        //DrawCircle(60, GetScreenHeight()-30, 10, button_color(state.down));
        //DrawCircle(90, GetScreenHeight()-60, 10, button_color(state.right));

        //int buttons_x = GetScreenWidth()-600;
        //int top_button_y = 200;

        //if(GuiButton((Rectangle){buttons_x, top_button_y, BUTTON_WIDTH, BUTTON_HEIGHT}, "Execute next instruction")) {
        //    computer_execute_next_instruction(&computer);
        //}

        //top_button_y += BUTTON_HEIGHT + BUTTON_SPACING;
        //GuiToggle((Rectangle){buttons_x, top_button_y, BUTTON_WIDTH, BUTTON_HEIGHT}, "toggle editor/computer view", &show_editor);
        //if(show_assembler_error) {
        //    if(!GuiMessageBox(rectangle_centered(300, 200), "Assembler error", last_assemble_status.error_message.data, "")) {
        //        show_assembler_error = false;
        //    }
        //}
        //if(show_editor) {
        //    if(up) move_cursor_up(&text_buffer, 1);
        //    if(down) move_cursor_down(&text_buffer, 1);

        //    top_button_y += BUTTON_HEIGHT + BUTTON_SPACING;
        //    if(GuiButton((Rectangle){buttons_x, top_button_y, BUTTON_WIDTH, BUTTON_HEIGHT}, "Assemble and load program")) {
        //        Arena *arena = arena_create();
        //        last_assemble_status = computer_assemble_and_load_program(&computer, read_entire_file_as_string(STR("../test.asm"), arena));
        //        if(!last_assemble_status.success) {
        //            show_assembler_error = true;
        //        }
        //        arena_free(arena);
        //    }

        //    int line_spacing = (FONT_SIZE+5);

        //    char cursor_char_as_cstr[] = { text_buffer.lines.data[text_buffer.cursor_line].data[text_buffer.cursor_char], '\0' };
        //    int char_width = MeasureText(cursor_char_as_cstr, FONT_SIZE);

        //    int x = GetScreenWidth() - 300;
        //    int y = 300;
        //    DrawRectangle(x, y+line_spacing*text_buffer.cursor_line, char_width, FONT_SIZE, DARKGRAY);
        //    for(size_t i = 0; i < text_buffer.lines.count; ++i) {
        //        DrawText(text_buffer.lines.data[i].data, x, y, FONT_SIZE, BLACK);
        //        y += line_spacing;
        //    }
        //} else {
        //    for(int i = 0; i < (int)ARRAY_LENGTH(computer.registers); ++i) {
        //        int x = GetScreenWidth()-200;
        //        int y = 30+(FONT_SIZE+5)*i;
        //        raylib_draw_uint16_hex(computer.registers[i], x, y, STR(""));
        //        //char buf[16];
        //        //snprintf(buf, sizeof buf, "0x%08X", computer.registers[i]);
        //        //int text_width = MeasureText(buf, FONT_SIZE);
        //        //DrawText(buf, GetScreenWidth()-text_width-30, 30+(FONT_SIZE+20*i), FONT_SIZE, BLACK);
        //    }

        //    for(int row = 0; row < 8; ++row) {
        //        uint16_t address = COMPUTER_PROGRAM_ENTRY_POINT + 8*row;

        //        int x = GetScreenWidth()-600;
        //        int y = GetScreenHeight()-(FONT_SIZE+5)*(8-row);
        //        raylib_draw_uint16_hex(address, x, y, STR(":"));
        //        for(int col = 0; col < 8; ++col) {
        //            String append = col == 7 ? STR("") : STR(",");
        //            Color color = address == computer.program_counter ? GREEN : BLACK;
        //            raylib_draw_uint8_hex(((uint8_t *)computer.memory_base)[address++], x + 70 + 50 * col, y, color, append);
        //        }
        //    }
        //}

        render_ui_layout(&state, ui_layout_state);
        DrawFPS(5, 5);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
