#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

typedef struct {
    char *data;
    size_t count;
    size_t capacity;
} DynamicString;

typedef struct {
    DynamicString *data;
    size_t count;
    size_t capacity;
} Lines;

typedef struct {
    Lines lines;
    size_t cursor_line;
    size_t cursor_char;
} TextBuffer;

TextBuffer load_file_into_text_buffer(String file_path);

void move_cursor_up(TextBuffer *buffer, uint32_t steps);
void move_cursor_down(TextBuffer *buffer, uint32_t steps);
void move_cursor_left(TextBuffer *buffer, uint32_t steps);
void move_cursor_right(TextBuffer *buffer, uint32_t steps);

#endif //TEXT_EDITOR_H
