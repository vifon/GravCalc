/** @file gravcalc.c
 *  @author Wojciech 'vifon' Siewierski
 */

/***********************************************************************************/
/* Copyright (C) 2015 Wojciech Siewierski <wojciech dot siewierski at onet dot pl> */
/*                                                                                 */
/* Author: Wojciech Siewierski <wojciech dot siewierski at onet dot pl>            */
/*                                                                                 */
/* This program is free software; you can redistribute it and/or                   */
/* modify it under the terms of the GNU General Public License                     */
/* as published by the Free Software Foundation; either version 3                  */
/* of the License, or (at your option) any later version.                          */
/*                                                                                 */
/* This program is distributed in the hope that it will be useful,                 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   */
/* GNU General Public License for more details.                                    */
/*                                                                                 */
/* You should have received a copy of the GNU General Public License               */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.            */
/***********************************************************************************/

#include "config.h"

#include <pebble.h>

static Window *s_main_window;

/** Number of keys on the calculator keypad. */
#define KEY_COUNT 16

/** The layer with the keys and their borders. */
static Layer *s_keypad_layer;
/** The layer with the current input, the stack state and their background. */
static Layer *s_input_layer;
/** The layer with the cursor. Has identical bounds as @ref s_input_layer. */
static Layer *s_cursor_layer;

/** Size of the calculator stack (@ref s_calculator_stack). */
#define CALC_STACK_SIZE 64
/** Numeric type used for the calculations.
 *
 *  TODO Switch to fixed-point as floats are not really supported.
 */
#define CALC_TYPE int
/** @p printf format specifier for @ref CALC_TYPE. */
#define CALC_TYPE_FMT "%d"
/** Calculations stack. */
static CALC_TYPE s_calculator_stack[CALC_STACK_SIZE];
/** Currently used stack slots in @ref s_calculator_stack. */
static unsigned int s_calculator_stack_index = 0;

/** Size of the input buffer (@ref s_input_buffer). */
#define INPUT_BUFFER_SIZE 32
/** The input buffer for the number. */
static char s_input_buffer[INPUT_BUFFER_SIZE] = "";
/** Currenly used space in the input buffer (@ref s_input_buffer) */
static size_t s_input_length = 0;
/** Flag marking whether inserting a comma should be allowed. */
static bool s_editing_fractional_part = false;

/** Width of the screen. */
#define SCREEN_W 144
/** Height of the screen minus the statusbar (168px - 16px). */
#define SCREEN_H 152

/** Height of the input box. */
#define INPUT_BOX_HEIGHT 28

/** The current position of the cursor. */
static GPoint s_cursor_position =
{SCREEN_W / 2,
 SCREEN_H / 2};

/** Text on the keypads. Only unique 1-character strings allowed. */
static const char s_keypad_text[][2] =
{"7", "8", "9", "+",
 "4", "5", "6", "-",
 "1", "2", "3", "*",
 "0", ".", "^", "/"};

/* Variables with colors for easy swapping. */
static const GColor main_color = GColorBlack;
static const GColor secondary_color = GColorClear;

/** @defgroup helpers
 *  @brief Helper functions.
 */

/** Convert a string to integer.
 *
 *  @param str String to convert.
 *  @param[out] endptr If non-NULL, set to the first invalid character.
 *
 *  @return The converted integer.
 */
static int str_to_int(const char *str, char **endptr) {
    int result = 0;

    int sign = 1;
    if (*str == '-') {
        ++str;
        sign = -1;
    }

    while (*str >= '0' && *str <= '9') {
        result *= 10;
        result += *str++ - '0';
    }

    /* save the position of the first invalid character */
    if (endptr != NULL) {
        /* http://stackoverflow.com/questions/993700/are-strtol-strtod-unsafe */
        *endptr = (char*)str;
    }

    return sign * result;
}

/** Convert a string to float.
 *
 *  @param str String to convert.
 *
 *  @return The converted float.
 */
static float str_to_float(const char *str) {
    /* TODO */
    return 0.0;
}

/** A simple implementation of the <tt>pow(3)</tt> standard function.
 *
 *  @param base
 *  @param exponent
 *
 *  @return The exponentiation result.
 *
 *  @note The exponent must not be negative!
 */
static CALC_TYPE my_pow(CALC_TYPE base, int exponent) {
    CALC_TYPE result = 1;

    while (exponent--) {
        result *= base;
    }

    return result;
}

/** Calculate the coordinates and bounds of the n-th calculator button
 *  relative to the upper upper left corner of the layer.
 *
 *  @param button_index Index number of the button.
 *
 *  @return GRect structure with the coordinates/bounds.
 */
static GRect get_rect_for_button(unsigned int button_index) {
    const unsigned int keypad_margin_x = 5;
    const unsigned int keypad_margin_y = 3;
    const unsigned int key_sep_x = 5;
    const unsigned int key_sep_y = 5;
    const unsigned int keys_in_row = 4;
    const unsigned int key_width = (SCREEN_W
                                    - key_sep_x * (keys_in_row-1)
                                    - keypad_margin_x * 2) / 4;
    const unsigned int key_height = 24;

    GRect bounds = GRect(
        /* horizontal position */
        button_index % keys_in_row
        * (key_width + key_sep_x)
        + keypad_margin_x,
        /* vertical position */
        button_index / keys_in_row
        * (key_height + key_sep_y)
        + keypad_margin_y,
        /* size */
        key_width,
        key_height);

    return bounds;
}

/** @}  */

/** @defgroup calculator Calculator functions
 *  @brief Calculator stack and input buffer management
 *  @{
 */

/** Change the edited fraction part (integral or fractional).
 *
 *  @param to_fractional If true, switch to the fractional part.
 *  Otherwise switch to the integral part.
 *
 *  @return False if the passed state was already set. True otherwise.
 */
static bool switch_edited_fraction_part(bool to_fractional) {
    if (s_editing_fractional_part == to_fractional) {
        return false;
    } else {
        s_editing_fractional_part = to_fractional;
        return true;
    }
}

/** Clear the whole input buffer and reset its state. */
static void clear_input() {
    s_input_length = 0;
    s_input_buffer[0] = '\0';
    switch_edited_fraction_part(false);
}

/** Push the passed number or the value in @ref s_input_buffer to the
 *  stack (@ref s_calculator_stack).
 *
 *  @param number Pointer to the number to be pushed. Pass NULL to
 *  read the value from the input buffer.
 *
 *  @return False if there is no space on the stack. True otherwise.
 */
static bool push_number(CALC_TYPE *number) {
    if (s_calculator_stack_index >= CALC_STACK_SIZE) {
        return false;
    }

    CALC_TYPE *slot = &s_calculator_stack[s_calculator_stack_index++];

    if (number == NULL) {
        *slot = str_to_int(s_input_buffer, NULL);
        clear_input();
    } else {
        *slot = *number;
    }

    return true;
}

/** Pop number from the stack and optionally return it to the editing buffer.
 */
static void pop_number(bool edit) {
    if (s_calculator_stack_index == 0) {
        return;
    }

    if (edit) {
        if (s_calculator_stack[s_calculator_stack_index-1] != 0) {
            s_input_length = snprintf(
                s_input_buffer, INPUT_BUFFER_SIZE,
                ""CALC_TYPE_FMT"",
                s_calculator_stack[s_calculator_stack_index-1]);
        } else {
            /* A lone leading 0 is still a leading 0 (which is invalid). */
            clear_input();
        }
    }
    --s_calculator_stack_index;
}

/** Perform an operation using the arguments from the calculator stack.
 *
 *  @param op An operator used to decide which operation to perform.
 *
 *  @return False if the stack was empty. True otherwise.
 */
static bool perform_operation(char op) {
    if (s_calculator_stack_index == 0) {
        return false;
    }

    CALC_TYPE lhs = s_calculator_stack[s_calculator_stack_index-1];
    CALC_TYPE rhs = str_to_int(s_input_buffer, NULL);

    CALC_TYPE result;
    switch (op) {
    case '+':
        result = lhs + rhs;
        break;
    case '-':
        result = lhs - rhs;
        break;
    case '*':
        result = lhs * rhs;
        break;
    case '/':
        result = lhs / rhs;
        break;
    case '^':
        if (rhs < 0) {
            /* negative exponents are not supported */
            return false;
        }
        result = my_pow(lhs, rhs);
        break;
    default:
        result = 0;
        break;
    }

    --s_calculator_stack_index;

#if ENABLE_AUTOPUSH
    push_number(&result);
    clear_input();
#else
    s_input_length = snprintf(s_input_buffer, INPUT_BUFFER_SIZE,
                              ""CALC_TYPE_FMT"", result);
#endif

    return true;
}

/** Add a new character to the input buffer (unless full).
 *
 *  @param new_character The character to append.
 */
static void validate_and_append_to_input_buffer(char new_character) {
    if (s_input_length+1 >= INPUT_BUFFER_SIZE) {
        return;                 /* the input buffer is full */
    }
    if (s_input_length == 0 && new_character == '0') {
        return;                 /* no leading zeros */
    }
    if (s_input_length == 0 && new_character == '.') {
        validate_and_append_to_input_buffer('.');
    }
    if (s_input_length != 0 && new_character == '-') {
        return;
    }

    s_input_buffer[s_input_length++] = new_character;
    s_input_buffer[s_input_length]   = '\0';
}

/** @}  */

/** @defgroup handlers Button handlers
 *  @brief Callbacks for the button presses
 *  @{
 */

/** Handler for the button used for selection/clicking.
 */
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    GPoint current_position = s_cursor_position;

    /* Iterate through all the keys. */
    unsigned int i;
    for (i = 0; i < KEY_COUNT; ++i) {
        GRect bounds = get_rect_for_button(i);

        /* Check if it is the key that was clicked. */
        if (grect_contains_point(&bounds, &current_position)) {
            const char* clicked_text = s_keypad_text[i];

            /* Check if it was the number key... */
            if (clicked_text[0] >= '0' && clicked_text[0] <= '9') {
                validate_and_append_to_input_buffer(clicked_text[0]);
            }
            /* ...or the decimal point... */
            else if (clicked_text[0] == '.') {
                if (switch_edited_fraction_part(true)) {
                    validate_and_append_to_input_buffer(clicked_text[0]);
                }
            }
            /* ...or operator. */
            else {
                switch (clicked_text[0]) {
                case '+':
                case '-':
                    /* If we're at the beginning of the buffer, just
                     * negate the number as the subtraction would be a
                     * NOOP anyway. The reverse operation works by
                     * accident thanks to this very property, when the
                     * AUTOPUSH is enabled. */
                    if (s_input_length == 0) {
                        validate_and_append_to_input_buffer('-');
                        break;
                    }
                case '*':
                case '/':
                case '^':
                    perform_operation(clicked_text[0]);
                    break;
                default:
                    APP_LOG(APP_LOG_LEVEL_DEBUG, "%s","Should never be reached.");
                }
            }
        }
    }
}

/** Handler for the button used for deleting the digits and poping the stack.
 */
static void cancel_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (s_input_length > 0) {
        --s_input_length;
        if (s_input_buffer[s_input_length] == '.') {
            switch_edited_fraction_part(false);
        }
        s_input_buffer[s_input_length] = '\0';
    } else {
        pop_number(true);
    }
}

/** Handler for the button used for clearing the whole input buffer.
 */
static void clear_input_click_handler(ClickRecognizerRef recognizer, void *context) {
    clear_input();
}

/** Handler for the button used for emptying the whole calculator stack.
 */
static void empty_stack_click_handler(ClickRecognizerRef recognizer, void *context) {
    clear_input();
    s_calculator_stack_index = 0;
}

/** Handler for the button used for pushing the current input to stack.
 */
static void push_click_handler(ClickRecognizerRef recognizer, void *context) {
    push_number(NULL);
}

/** @} */

/** Set the button handlers.
 *
 *  <b>Upper</b>: backspace / pop from the stack<br />
 *  <b>Upper long</b>: clear the current input<br />
 *  <b>Middle</b>: push to the stack<br />
 *  <b>Middle long</b>: empty the stack<br />
 *  <b>Lower</b>: click / confirm<br />
 */
static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, cancel_click_handler);
    window_long_click_subscribe(BUTTON_ID_UP, 500, clear_input_click_handler, NULL);

    window_long_click_subscribe(BUTTON_ID_SELECT, 1000, empty_stack_click_handler, NULL);
    window_single_click_subscribe(BUTTON_ID_SELECT, push_click_handler);

    window_single_click_subscribe(BUTTON_ID_DOWN, select_click_handler);
}

/** @defgroup redraw
 *  @brief Callbacks for redrawing the display.
 *  @{
 */

/** Draw the keys and their borders.
 */
static void draw_keypad_callback(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, main_color);
    graphics_context_set_stroke_color(ctx, secondary_color);
    graphics_context_set_text_color(ctx, secondary_color);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 2, GCornerNone);

    const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

    unsigned int i;
    for (i = 0; i < KEY_COUNT; ++i) {
        GRect bounds = get_rect_for_button(i);

        bool active = grect_contains_point(&bounds, &s_cursor_position);

        if (active) {
            /* invert colors */
            graphics_context_set_text_color(ctx, main_color);
            graphics_context_set_fill_color(ctx, secondary_color);
            graphics_fill_rect(ctx, bounds, 1, GCornerNone);
        } else {
            graphics_draw_rect(ctx, bounds);
        }

        graphics_draw_text(
            ctx,
            s_keypad_text[i],
            font,
            bounds,
            GTextOverflowModeTrailingEllipsis,
            GTextAlignmentCenter,
            NULL);
        if (active) {
            /* cleanup */
            graphics_context_set_text_color(ctx, secondary_color);
        }
    }
}

/** Draw the current input the stack information and their background.
 */
static void draw_input_callback(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, secondary_color);
    graphics_context_set_text_color(ctx, main_color);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 2, GCornerNone);

    char buffer[64];
    switch (s_calculator_stack_index) {
    case 0:
        snprintf(buffer, 64,
                 "%s",
                 s_input_length > 0 ? s_input_buffer : "0");
        break;
    case 1:
        snprintf(buffer, 64,
                 ""CALC_TYPE_FMT" %s %s",
                 s_calculator_stack[s_calculator_stack_index-1],
                 "_",
                 s_input_length > 0 ? s_input_buffer : "0");
        break;
    case 2:
        snprintf(buffer, 64,
                 ""CALC_TYPE_FMT"  "CALC_TYPE_FMT" %s %s",
                 s_calculator_stack[s_calculator_stack_index-2],
                 s_calculator_stack[s_calculator_stack_index-1],
                 "_",
                 s_input_length > 0 ? s_input_buffer : "0");
        break;
    default:
        snprintf(buffer, 64,
                 "[%u]... "CALC_TYPE_FMT"  "CALC_TYPE_FMT" %s %s",
                 s_calculator_stack_index,
                 s_calculator_stack[s_calculator_stack_index-2],
                 s_calculator_stack[s_calculator_stack_index-1],
                 "_",
                 s_input_length > 0 ? s_input_buffer : "0");
        break;
    }

    /* create the text margin */
    GRect bounds = layer_get_bounds(layer);
    bounds.origin.x += 5;
    bounds.size.w -= 10;

    graphics_draw_text(
        ctx,
        buffer,
        fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
        bounds,
        GTextOverflowModeTrailingEllipsis,
        GTextAlignmentRight,
        NULL);
}

/** Draw the cursor with an outline. */
static void draw_cursor_callback(Layer *layer, GContext *ctx) {
    /* Draw the cursor. */
    graphics_context_set_fill_color(ctx, secondary_color);
    graphics_fill_circle(ctx, s_cursor_position, 3);

    /* Draw the cursor outline for better visibility. */
    graphics_context_set_stroke_color(ctx, main_color);
    graphics_draw_circle(ctx, s_cursor_position, 4);
}

/** @} */

/** @defgroup window Window management
 *  @brief Window and layer management
 *  @{
 */

static void main_window_load(Window *window) {
    /* Create the layer with the keypad etc. */
    s_keypad_layer = layer_create(
        GRect(0, INPUT_BOX_HEIGHT,
              144, SCREEN_H-INPUT_BOX_HEIGHT));
    layer_add_child(
        window_get_root_layer(window),
        s_keypad_layer);
    layer_set_update_proc(
        s_keypad_layer,
        draw_keypad_callback);

    /* Create the layer with the input box. */
    s_input_layer = layer_create(
        GRect(0, 0,
              144, INPUT_BOX_HEIGHT));
    layer_add_child(
        window_get_root_layer(window),
        s_input_layer);
    layer_set_update_proc(
        s_input_layer,
        draw_input_callback);

    /* Create the topmost layer with the cursor. */
    s_cursor_layer = layer_create(GRect(0, INPUT_BOX_HEIGHT,
                                        144, SCREEN_H-INPUT_BOX_HEIGHT));
    layer_add_child(
        window_get_root_layer(window),
        s_cursor_layer);
    layer_set_update_proc(
        s_cursor_layer,
        draw_cursor_callback);
}

static void main_window_unload(Window *window) {
    layer_destroy(s_keypad_layer);
    layer_destroy(s_input_layer);
    layer_destroy(s_cursor_layer);
}

/** Read the data from the accelerometer and then move the cursor
 *  (@ref s_cursor_position) according to them.
 */
static void read_accel_and_move_cursor_callback(AccelData *data, uint32_t num_samples) {
    s_cursor_position.x +=  data[0].x * (SCREEN_W / 4000.f);
    s_cursor_position.y += -data[0].y * (SCREEN_H / 4000.f);

    if (s_cursor_position.x < 0) {
        s_cursor_position.x = 0;
    } else if (s_cursor_position.x > SCREEN_W) {
        s_cursor_position.x = SCREEN_W;
    }

    if (s_cursor_position.y < 0) {
        s_cursor_position.y = 0;
    } else if (s_cursor_position.y > SCREEN_H-INPUT_BOX_HEIGHT) {
        s_cursor_position.y = SCREEN_H-INPUT_BOX_HEIGHT;
    }

    layer_mark_dirty(s_cursor_layer);
}

static void init() {
    // Create main Window
    s_main_window = window_create();
    window_set_click_config_provider(s_main_window, click_config_provider);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    window_stack_push(s_main_window, true);

    // Subscribe to the accelerometer data service
    int num_samples = 1;
    accel_data_service_subscribe(num_samples, read_accel_and_move_cursor_callback);

    // Choose update rate
    accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
}

static void deinit() {
    // Destroy main Window
    window_destroy(s_main_window);

    accel_data_service_unsubscribe();
}

/** @} */

int main(void) {
    init();
    app_event_loop();
    deinit();
}
