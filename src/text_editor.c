#include "text_editor.h"

DynamicString copy_string_to_new_dynamic_string(String string) {
    DynamicString ret = {};
    ret.count = string.length;
    size_t initial_capacity = string.length;
    ret.capacity = initial_capacity;
    ret.data = malloc(ret.capacity);
    assert(ret.data && "failed to malloc");
    memcpy(ret.data, string.data, string.length);
    return ret;
}

TextBuffer load_file_into_text_buffer(String file_path) {
    TextBuffer ret = {};
    Arena *temp_arena = arena_create();
    String file = read_entire_file_as_string(file_path, temp_arena);
    size_t line_count = 0;
    String *lines = split_string(file, '\n', &line_count, false, temp_arena);

    for(size_t i = 0; i < line_count; ++i) {
        da_append(&ret.lines, copy_string_to_new_dynamic_string(lines[i]));
    }

    arena_free(temp_arena);

    return ret;
}

void move_cursor_up(TextBuffer *buffer, uint32_t steps) {
    if(steps >= buffer->cursor_line)
        buffer->cursor_line = 0;
    else
        buffer->cursor_line -= steps;
}
void move_cursor_down(TextBuffer *buffer, uint32_t steps) {
    if(steps < buffer->lines.count - buffer->cursor_line)
        buffer->cursor_line += steps;
}
void move_cursor_left(TextBuffer *buffer, uint32_t steps) {
}
void move_cursor_right(TextBuffer *buffer, uint32_t steps) {
}

// TODO: free everything at once, some kind of custom allocator?
void unload_text_buffer(TextBuffer text_buffer) {
    for(size_t i = 0; i < text_buffer.lines.count; ++i) {
        free(text_buffer.lines.data[i].data);
    }
    free(text_buffer.lines.data);
}
