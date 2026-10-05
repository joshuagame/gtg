#include "ui/console.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static const double console_characters_per_second = 72.0;

static size_t console_copy_string(char *destination, size_t capacity, const char *source)
{
    size_t length = 0U;

    if (destination == NULL || capacity == 0U)
    {
        return 0U;
    }

    if (source != NULL)
    {
        while (length + 1U < capacity && source[length] != '\0')
        {
            destination[length] = source[length];
            length += 1U;
        }
    }

    destination[length] = '\0';
    return length;
}

static void console_reveal_all(Console *console)
{
    for (size_t index = 0U; index < console->output_count; index += 1U)
    {
        const size_t slot = (console->output_start + index) % CONSOLE_OUTPUT_CAPACITY;
        console->output[slot].visible_length = console->output[slot].length;
    }
    console->reveal_credit = 0.0;
}

static void console_clear_output(Console *console)
{
    console->output_start = 0U;
    console->output_count = 0U;
    console->reveal_credit = 0.0;
}

static void console_write_line(Console *console, const char *text)
{
    size_t slot = 0U;

    if (console->output_count < CONSOLE_OUTPUT_CAPACITY)
    {
        slot = (console->output_start + console->output_count) % CONSOLE_OUTPUT_CAPACITY;
        console->output_count += 1U;
    }
    else
    {
        slot = console->output_start;
        console->output_start = (console->output_start + 1U) % CONSOLE_OUTPUT_CAPACITY;
    }

    ConsoleLine *line = &console->output[slot];
    line->length = console_copy_string(line->text, sizeof(line->text), text);
    line->visible_length = console->typing_enabled ? 0U : line->length;
}

static void console_clear_input(Console *console)
{
    console->input[0] = '\0';
    console->input_length = 0U;
}

static void console_append_input(Console *console, const ConsoleInput *input)
{
    for (size_t index = 0U; index < input->text_length; index += 1U)
    {
        const unsigned char character = (unsigned char)input->text[index];

        if (!isprint(character) || console->input_length + 1U >= sizeof(console->input))
        {
            continue;
        }

        console->input[console->input_length] = (char)character;
        console->input_length += 1U;
        console->input[console->input_length] = '\0';
    }
}

static void console_store_history(Console *console)
{
    if (console->input_length == 0U)
    {
        return;
    }

    if (console->history_count == CONSOLE_HISTORY_CAPACITY)
    {
        for (size_t index = 1U; index < CONSOLE_HISTORY_CAPACITY; index += 1U)
        {
            (void)console_copy_string(console->history[index - 1U],
                                      sizeof(console->history[index - 1U]),
                                      console->history[index]);
        }
        console->history_count -= 1U;
    }

    (void)console_copy_string(console->history[console->history_count],
                              sizeof(console->history[console->history_count]), console->input);
    console->history_count += 1U;
    console->history_cursor = console->history_count;
}

static void console_recall_history(Console *console, bool previous)
{
    if (console->history_count == 0U)
    {
        return;
    }

    if (previous)
    {
        if (console->history_cursor > 0U)
        {
            console->history_cursor -= 1U;
        }
    }
    else if (console->history_cursor < console->history_count)
    {
        console->history_cursor += 1U;
    }

    if (console->history_cursor == console->history_count)
    {
        console_clear_input(console);
        return;
    }

    console->input_length = console_copy_string(console->input, sizeof(console->input),
                                                console->history[console->history_cursor]);
}

static void console_normalize_command(const Console *console, char *command, size_t capacity)
{
    size_t begin = 0U;
    size_t end = console->input_length;
    size_t output_length = 0U;

    while (begin < end && isspace((unsigned char)console->input[begin]))
    {
        begin += 1U;
    }

    while (end > begin && isspace((unsigned char)console->input[end - 1U]))
    {
        end -= 1U;
    }

    while (begin < end && output_length + 1U < capacity)
    {
        command[output_length] = (char)toupper((unsigned char)console->input[begin]);
        output_length += 1U;
        begin += 1U;
    }
    command[output_length] = '\0';
}

static ConsoleAction console_execute(Console *console)
{
    char command[CONSOLE_INPUT_CAPACITY] = {0};
    char unknown_message[CONSOLE_LINE_CAPACITY] = {0};
    console_normalize_command(console, command, sizeof(command));

    if (command[0] == '\0')
    {
        return CONSOLE_ACTION_NONE;
    }

    console_store_history(console);

    if (strcmp(command, "HELP") == 0)
    {
        console_write_line(console, "AVAILABLE COMMANDS:");
        console_write_line(console, "  HELP  - LIST COMMANDS");
        console_write_line(console, "  CLEAR - CLEAR TERMINAL OUTPUT");
        console_write_line(console, "  ABOUT - DISPLAY SYSTEM INFORMATION");
        console_write_line(console, "  QUIT  - TERMINATE SESSION");
    }
    else if (strcmp(command, "CLEAR") == 0)
    {
        console_clear_output(console);
    }
    else if (strcmp(command, "ABOUT") == 0)
    {
        console_write_line(console, "WOPR // GLOBAL THERMONUCLEAR WAR");
        console_write_line(console, "GTG STUDY SYSTEM // C17 + RAYLIB");
    }
    else if (strcmp(command, "QUIT") == 0)
    {
        console_write_line(console, "SESSION TERMINATED.");
        return CONSOLE_ACTION_QUIT;
    }
    else
    {
        (void)snprintf(unknown_message, sizeof(unknown_message), "UNKNOWN COMMAND: %.108s",
                       command);
        console_write_line(console, unknown_message);
    }

    return CONSOLE_ACTION_NONE;
}

void console_init(Console *console)
{
    if (console == NULL)
    {
        return;
    }

    *console = (Console){0};
    console->typing_enabled = true;
    console_write_line(console, "GREETINGS PROFESSOR FALKEN.");
    console_write_line(console, "SHALL WE PLAY A GAME?");
    console_write_line(console, "TYPE HELP FOR AVAILABLE COMMANDS.");
}

ConsoleAction console_handle_input(Console *console, const ConsoleInput *input)
{
    ConsoleAction action = CONSOLE_ACTION_NONE;

    if (console == NULL || input == NULL || input->text_length > sizeof(input->text))
    {
        return action;
    }

    if (input->toggle_typing)
    {
        console_set_typing_enabled(console, !console->typing_enabled);
    }

    if (input->history_previous)
    {
        console_recall_history(console, true);
    }
    else if (input->history_next)
    {
        console_recall_history(console, false);
    }

    console_append_input(console, input);

    if (input->backspace && console->input_length > 0U)
    {
        console->input_length -= 1U;
        console->input[console->input_length] = '\0';
    }

    if (input->submit)
    {
        action = console_execute(console);
        console_clear_input(console);
        console->history_cursor = console->history_count;
    }

    return action;
}

void console_update_animation(Console *console, double frame_seconds)
{
    bool has_hidden_characters = false;

    if (console == NULL || !console->typing_enabled || !isfinite(frame_seconds) ||
        frame_seconds <= 0.0)
    {
        return;
    }

    console->reveal_credit += frame_seconds * console_characters_per_second;

    for (size_t index = 0U; index < console->output_count && console->reveal_credit >= 1.0;
         index += 1U)
    {
        const size_t slot = (console->output_start + index) % CONSOLE_OUTPUT_CAPACITY;
        ConsoleLine *line = &console->output[slot];

        while (line->visible_length < line->length && console->reveal_credit >= 1.0)
        {
            line->visible_length += 1U;
            console->reveal_credit -= 1.0;
        }
    }

    for (size_t index = 0U; index < console->output_count; index += 1U)
    {
        const size_t slot = (console->output_start + index) % CONSOLE_OUTPUT_CAPACITY;
        if (console->output[slot].visible_length < console->output[slot].length)
        {
            has_hidden_characters = true;
            break;
        }
    }

    if (!has_hidden_characters)
    {
        console->reveal_credit = 0.0;
    }
}

void console_set_typing_enabled(Console *console, bool enabled)
{
    if (console == NULL)
    {
        return;
    }

    console->typing_enabled = enabled;
    if (!enabled)
    {
        console_reveal_all(console);
    }
}

bool console_typing_enabled(const Console *console)
{
    return console != NULL && console->typing_enabled;
}

const char *console_input_text(const Console *console)
{
    return console != NULL ? console->input : "";
}

size_t console_input_length(const Console *console)
{
    return console != NULL ? console->input_length : 0U;
}

size_t console_output_count(const Console *console)
{
    return console != NULL ? console->output_count : 0U;
}

const ConsoleLine *console_output_line(const Console *console, size_t index)
{
    if (console == NULL || index >= console->output_count)
    {
        return NULL;
    }

    const size_t slot = (console->output_start + index) % CONSOLE_OUTPUT_CAPACITY;
    return &console->output[slot];
}
