#ifndef GTG_UI_CONSOLE_H
#define GTG_UI_CONSOLE_H

#include <stdbool.h>
#include <stddef.h>

enum
{
    CONSOLE_INPUT_CAPACITY = 128,
    CONSOLE_FRAME_TEXT_CAPACITY = 32,
    CONSOLE_LINE_CAPACITY = 128,
    CONSOLE_OUTPUT_CAPACITY = 32,
    CONSOLE_HISTORY_CAPACITY = 8,
};

typedef enum
{
    CONSOLE_ACTION_NONE = 0,
    CONSOLE_ACTION_QUIT,
} ConsoleAction;

typedef struct
{
    char text[CONSOLE_FRAME_TEXT_CAPACITY];
    size_t text_length;
    bool backspace;
    bool submit;
    bool history_previous;
    bool history_next;
    bool toggle_typing;
} ConsoleInput;

typedef struct
{
    char text[CONSOLE_LINE_CAPACITY];
    size_t length;
    size_t visible_length;
} ConsoleLine;

typedef struct
{
    char input[CONSOLE_INPUT_CAPACITY];
    size_t input_length;
    ConsoleLine output[CONSOLE_OUTPUT_CAPACITY];
    size_t output_start;
    size_t output_count;
    char history[CONSOLE_HISTORY_CAPACITY][CONSOLE_INPUT_CAPACITY];
    size_t history_count;
    size_t history_cursor;
    double reveal_credit;
    bool typing_enabled;
} Console;

void console_init(Console *console);
ConsoleAction console_handle_input(Console *console, const ConsoleInput *input);
void console_update_animation(Console *console, double frame_seconds);
void console_set_typing_enabled(Console *console, bool enabled);
bool console_typing_enabled(const Console *console);
const char *console_input_text(const Console *console);
size_t console_input_length(const Console *console);
size_t console_output_count(const Console *console);
const ConsoleLine *console_output_line(const Console *console, size_t index);

#endif
