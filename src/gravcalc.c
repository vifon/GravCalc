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

#include "fixed.h"

static Window *s_main_window;

/** Number of keys on the calculator keypad. */
#define KEY_COUNT 16

/** Number of switchable keypads */
#define KEYPAD_COUNT 1

/** Text on the keypads. Only unique 1-character strings allowed. */
static const char s_keypad_text[KEYPAD_COUNT][KEY_COUNT][2] =
{{"7", "8", "9", "+",
  "4", "5", "6", "-",
  "1", "2", "3", "*",
  "0", ".", "^", "/"}};

/** Index of the currently used keypad. */
static size_t s_current_keypad = 0;

/** The layer with the keys and their borders. */
static Layer *s_keypad_layer;
/** The layer with the current input, the stack state and their background. */
static Layer *s_input_layer;
/** The layer with the cursor. Has identical bounds as @ref s_keypad_layer. */
static Layer *s_cursor_layer;

/** A pointer to the button currently focused with the cursor. Used to
 *  not search for that button multiple times per frame, once in every
 *  hook. It is set in @ref draw_keypad_callback and used in other
 *  callbacks and handlers. <b>Is considered valid only if @ref
 *  s_focused_button_index is not equal -1</b>.
 */
static GRect s_focused_button;

/** Index of the button pointed by @ref s_focused_button.
 *  -1 means it wasn't yet set.
 */
static int s_focused_button_index = -1;

/** Calculations stack. */
static CALC_TYPE s_calculator_stack[CALC_STACK_SIZE];
/** Currently used stack slots in @ref s_calculator_stack. */
static unsigned int s_calculator_stack_index = 0;

/** The input buffer for the number. */
static char s_input_buffer[INPUT_BUFFER_SIZE] = "";
/** Currenly used space in the input buffer (@ref s_input_buffer) */
static size_t s_input_length = 0;
/** Flag marking whether inserting a comma should be allowed. */
static bool s_editing_fractional_part = false;

/** Pointer to the error message. */
static const char* s_error_msg = 0;

/** Width of the screen. */
#define SCREEN_W 144
/** Height of the screen minus the statusbar (168px - 16px). */
#define SCREEN_H 152

/** Height of the input box. */
#define INPUT_BOX_HEIGHT 28

/** Height of the keypad. */
#define KEYPAD_HEIGHT ((SCREEN_H) - (INPUT_BOX_HEIGHT))

/** The current position of the cursor. */
static GPoint s_cursor_position =
{SCREEN_W / 2,
 KEYPAD_HEIGHT / 2};

/** Calculate the coordinates and bounds of the n-th calculator button
 *  relative to the upper upper left corner of the layer.
 *
 *  @param button_index Index number of the button.
 *
 *  @return GRect structure with the coordinates/bounds.
 */
static GRect get_rect_for_button(unsigned int button_index) {
    const unsigned int keypad_margin_x = 5;
    const unsigned int keypad_margin_y = 4;
    const unsigned int key_sep_x = 5;
    const unsigned int key_sep_y = 5;
    const unsigned int keys_in_row = 4;
    const unsigned int usable_screen_width =
        SCREEN_W
        - key_sep_x * (keys_in_row-1)
        - keypad_margin_x * 2;
    const float key_width =    /* 0.5 added for the proper rounding */
        ((float)usable_screen_width / keys_in_row) + 0.5;
    const unsigned int key_height = 25;

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

/** Switch to the next keypad.
 */
static void keypad_next() {
    s_current_keypad = (s_current_keypad + 1) % KEYPAD_COUNT;
}

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

/** Set the error message to be shown.
 *
 *  @param msg Error message. Pass NULL to disable.
 */
static void set_error(const char* msg) {
    s_error_msg = msg;
}

/** Push the passed number or the value in @ref s_input_buffer to the
 *  stack (@ref s_calculator_stack).
 *
 *  @param number Pointer to the number to be pushed. Pass NULL to
 *  read the value from the input buffer.
 *
 *  @return False if there is no space on the stack or the value is
 *  out of range of the internal number representation. True
 *  otherwise.
 */
static bool push_number(CALC_TYPE *number) {
    if (s_calculator_stack_index >= CALC_STACK_SIZE) {
        return false;
    }

    CALC_TYPE *slot = &s_calculator_stack[s_calculator_stack_index++];

    if (number == NULL) {
        bool overflow = false;
        *slot = str_to_fixed(s_input_buffer, &overflow);
        if (overflow) {
            set_error("OUT OF RANGE");
            --s_calculator_stack_index;
            return false;
        }
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
            char tmp[64];
            REPR(s_calculator_stack[s_calculator_stack_index-1], tmp, 64);
            s_input_length = snprintf(
                s_input_buffer, INPUT_BUFFER_SIZE,
                ""CALC_TYPE_FMT"",
                tmp);
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
 *  @return False if the operation has been performed successfully.
 */
static bool perform_operation(char op) {
    if (s_calculator_stack_index == 0) {
        return false;
    }

    bool overflow = false;

    CALC_TYPE lhs = s_calculator_stack[s_calculator_stack_index-1];
    CALC_TYPE rhs = str_to_fixed(s_input_buffer, &overflow);

    if (overflow) {
        set_error("OVERFLOW");
        return false;
    }

    CALC_TYPE result;
    switch (op) {
    case '+':
        result = ADD(lhs, rhs, &overflow);
        break;
    case '-':
        result = SUBT(lhs, rhs, &overflow);
        break;
    case '*':
        result = MULT(lhs, rhs, &overflow);
        break;
    case '/':
        result = DIV(lhs, rhs);
        break;
    case '^':
        result = POW(lhs, fixed_to_int(rhs), &overflow);
        break;
    default:
        result = 0;
        break;
    }

    if (overflow) {
        set_error("OVERFLOW");
        return false;
    }

    --s_calculator_stack_index;

#if ENABLE_AUTOPUSH
    push_number(&result);
    clear_input();
#else
    s_input_length = strlen(REPR(result, s_input_buffer, INPUT_BUFFER_SIZE));
#endif

    return true;
}

/** Add a new character to the input buffer without any validation.
 *
 *  @param new_character The character to append.
 *
 *  @note Should never be called directly. Rather call @ref
 *  validate_and_append_to_input_buffer.
 */
static void append_to_input_buffer(char new_character) {
    s_input_buffer[s_input_length++] = new_character;
    s_input_buffer[s_input_length]   = '\0';
}

/** Validate and perhaps add a new character to the input buffer.
 *
 *  Validation:
 *  - input buffer cannot be full,
 *  - no leading zeros allowed...,
 *  - ...unless just before the decimal point, in which case it is
 *    automatically added,
 *  - minus sign allowed only at the beginning.
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
    if (new_character == '.') {
        /* Ugny corner cases: Inserting '.' at the beginning of the
         * buffer or just after a minus sign should automatically
         * insert a zero. The deleting function must take case of that
         * case too. */
        if ((s_input_length == 0) ||
            (s_input_length == 1 &&
             s_input_buffer[s_input_length-1] == '-')) {

            append_to_input_buffer('0');
        }
    }
    if (s_input_length != 0 && new_character == '-') {
        return;
    }

    append_to_input_buffer(new_character);
}

/** Delete a single character from the input buffer.
 *
 *  @note In case of the string "0." it deletes two charactes to
 *  prevent a lone leading zero.
 *
 *  @note It does @b not check whether the input buffer is empty.
 */
static void delete_from_input_buffer() {
    if (s_input_buffer[--s_input_length] == '.') {
        if ((s_input_length == 1 && s_input_buffer[s_input_length-1] == '0') ||
            (s_input_length == 2 &&
             s_input_buffer[s_input_length-1] == '0' &&
             s_input_buffer[s_input_length-2] == '-')) {
            /* Ugly corner case: inserting "0." and deleting "." would
             * allow a leading zero. Delete both to prevent it. Do the
             * same with "-0." too. */
            --s_input_length;
        }
        switch_edited_fraction_part(false);
    }
    s_input_buffer[s_input_length] = '\0';
}

/** Perform the operation associated with the clicked button.
 *
 *  @param button_text The single character displayed on the button.
 */
static void click_button(char button_text) {
    /* Check if it was the number key... */
    if (button_text >= '0' && button_text <= '9') {
        validate_and_append_to_input_buffer(button_text);
    }
    /* ...or the decimal point... */
    else if (button_text == '.') {
        if (switch_edited_fraction_part(true)) {
            validate_and_append_to_input_buffer(button_text);
        }
    }
    /* ...or operator. */
    else {
        switch (button_text) {
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
        case '+':
        case '*':
        case '/':
        case '^':
            perform_operation(button_text);
            break;
        case ' ':
            /* ignore */
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_DEBUG, "%s","Should never be reached.");
        }
    }
}

/** @}  */

/** @defgroup handlers Button handlers
 *  @brief Callbacks for the button presses
 *  @{
 */

/** Handler for the button used for selection/clicking.
 */
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (s_focused_button_index != -1) {
        set_error(NULL);
        char clicked_text = s_keypad_text[s_current_keypad][s_focused_button_index][0];
        click_button(clicked_text);
    }
}

/** Handler for the button used for deleting the digits and poping the stack.
 */
static void cancel_click_handler(ClickRecognizerRef recognizer, void *context) {
    set_error(NULL);
    if (s_input_length > 0) {
        delete_from_input_buffer();
    } else {
        pop_number(true);
    }
}

/** Handler for the button used for clearing the whole input buffer.
 */
static void clear_input_click_handler(ClickRecognizerRef recognizer, void *context) {
    set_error(NULL);
    clear_input();
}

/** Handler for the button used for emptying the whole calculator stack.
 */
static void empty_stack_click_handler(ClickRecognizerRef recognizer, void *context) {
    set_error(NULL);
    clear_input();
    s_calculator_stack_index = 0;
}

/** Handler for the button used for pushing the current input to stack.
 */
static void push_click_handler(ClickRecognizerRef recognizer, void *context) {
    set_error(NULL);
    push_number(NULL);
}

/** Handler for the button used switching the used keypad.
 */
static void switch_keypad_handler(ClickRecognizerRef recognizer, void *context) {
    set_error(NULL);
    keypad_next();
}

/** @} */

/** Set the button handlers.
 *
 *  <b>Upper</b>: backspace / pop from the stack<br />
 *  <b>Upper long</b>: clear the current input<br />
 *  <b>Middle</b>: push to the stack<br />
 *  <b>Middle long</b>: empty the stack<br />
 *  <b>Lower</b>: click / confirm<br />
 *  <b>Lower long</b>: switch the keypad<br />
 */
static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, cancel_click_handler);
    window_long_click_subscribe(BUTTON_ID_UP, 500, clear_input_click_handler, NULL);

    window_single_click_subscribe(BUTTON_ID_SELECT, push_click_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT, 1000, empty_stack_click_handler, NULL);

    window_single_click_subscribe(BUTTON_ID_DOWN, select_click_handler);
    window_long_click_subscribe(BUTTON_ID_DOWN, 500, switch_keypad_handler, NULL);
}

/** @defgroup redraw
 *  @brief Callbacks for redrawing the display.
 *  @{
 */

/** Draw the keys and their borders.
 */
static void draw_keypad_callback(Layer *layer, GContext *ctx) {
    const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

    unsigned int i;
    for (i = 0; i < KEY_COUNT; ++i) {
        /* ignore the keys marked with a space */
        if (s_keypad_text[s_current_keypad][i][0] != ' ') {

            GRect bounds = get_rect_for_button(i);

            bool active = grect_contains_point(&bounds, &s_cursor_position);

            if (active) {
                s_focused_button = bounds;
                s_focused_button_index = i;

                graphics_context_set_text_color(ctx, COLOR_BUTTON_FOCUSED_TEXT);
                graphics_context_set_fill_color(ctx, COLOR_BUTTON_FOCUSED_BG);
                graphics_context_set_stroke_color(ctx, COLOR_BUTTON_FOCUSED_BORDER);
            } else {
                graphics_context_set_text_color(ctx, COLOR_BUTTON_TEXT);
                graphics_context_set_fill_color(ctx, COLOR_BUTTON_BG);
                graphics_context_set_stroke_color(ctx, COLOR_BUTTON_BORDER);
            }
            graphics_fill_rect(ctx, bounds, 1, GCornerNone);
            graphics_draw_rect(ctx, bounds);

            graphics_draw_text(
                ctx,
                s_keypad_text[s_current_keypad][i],
                font,
                bounds,
                GTextOverflowModeTrailingEllipsis,
                GTextAlignmentCenter,
                NULL);
        }
    }
}

/** Draw the current input the stack information and the background.
 *  Additionally display the error message, if any.
 */
static void draw_input_callback(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, COLOR_DISPLAY_BG);
    graphics_context_set_text_color(ctx, COLOR_DISPLAY_TEXT);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 2, GCornerNone);

    char buffer[64];
    switch (s_calculator_stack_index) {
        char lhs[32];
        char rhs[32];
    case 0:
        snprintf(buffer, sizeof(buffer),
                 "%s",
                 s_input_length > 0 ? s_input_buffer : "0");
        break;
    case 1:
        REPR(s_calculator_stack[s_calculator_stack_index-1], lhs, sizeof(lhs));
        snprintf(buffer, sizeof(buffer),
                 ""CALC_TYPE_FMT" %s %s",
                 lhs, "_",
                 s_input_length > 0 ? s_input_buffer : "0");
        break;
    case 2:
        REPR(s_calculator_stack[s_calculator_stack_index-1], lhs, sizeof(lhs));
        REPR(s_calculator_stack[s_calculator_stack_index-2], rhs, sizeof(rhs));
        snprintf(buffer, sizeof(buffer),
                 ""CALC_TYPE_FMT"  "CALC_TYPE_FMT" %s %s",
                 rhs, lhs, "_",
                 s_input_length > 0 ? s_input_buffer : "0");
        break;
    default:
        REPR(s_calculator_stack[s_calculator_stack_index-1], lhs, sizeof(lhs));
        REPR(s_calculator_stack[s_calculator_stack_index-2], rhs, sizeof(rhs));
        snprintf(buffer, sizeof(buffer),
                 "[%u]... "CALC_TYPE_FMT"  "CALC_TYPE_FMT" %s %s",
                 s_calculator_stack_index,
                 rhs, lhs, "_",
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

    if (s_error_msg) {
        GRect bounds = layer_get_bounds(layer);
        bounds.origin.x += 5;
        bounds.origin.y -= 4;
        bounds.size.w -= 10;

        graphics_draw_text(
            ctx,
            s_error_msg,
            fonts_get_system_font(FONT_KEY_GOTHIC_14),
            bounds,
            GTextOverflowModeTrailingEllipsis,
            GTextAlignmentLeft,
            NULL);
    }
}

/** Draw the cursor with an outline. */
static void draw_cursor_callback(Layer *layer, GContext *ctx) {
    /* Draw the cursor. */
    graphics_context_set_fill_color(ctx, COLOR_CURSOR);
    graphics_fill_circle(ctx, s_cursor_position, 3);

    /* Draw the cursor outline for better visibility. */
    graphics_context_set_stroke_color(ctx, COLOR_CURSOR_BORDER);
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
              144, KEYPAD_HEIGHT));
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
    s_cursor_layer = layer_create(
        GRect(0, INPUT_BOX_HEIGHT,
              144, KEYPAD_HEIGHT));
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
 *
 *  @note On the first @ref CALIBRATION_SAMPLES calls only calibrate
 *  the balance point of the accelerometer by calculating the average
 *  value from them.
 */
static void read_accel_and_move_cursor_callback(AccelData *data, uint32_t num_samples) {
    static int samples_until_calibrated = CALIBRATION_SAMPLES;
    static int zero_x = 0;
    static int zero_y = 0;

    /* collect the sample for calibration */
    if (samples_until_calibrated > 0) {
        --samples_until_calibrated;
        zero_x += data[0].x;
        zero_y += data[0].y;
        return;
    }

    /* all samples collected, calculate the average */
    if (samples_until_calibrated == 0) {
        --samples_until_calibrated;
        zero_x /= CALIBRATION_SAMPLES;
        zero_y /= CALIBRATION_SAMPLES;
    }

    /* the button is concave, simulate its steepness */
    int x_slope = 0;
    int y_slope = 0;
    if (s_focused_button_index != -1) {
        GPoint center = grect_center_point(&s_focused_button);
        x_slope = (center.x - s_cursor_position.x);
        y_slope = (center.y - s_cursor_position.y);
    }

    /* apply the new position cursor */
    const float ACCEL_MAX = 4000.f;
    s_cursor_position.x +=
        ((data[0].x - zero_x) * (SCREEN_W / ACCEL_MAX)
         + x_slope / STEEPNESS_FACTOR);
    s_cursor_position.y +=
        (-(data[0].y - zero_y) * (SCREEN_H / ACCEL_MAX)
         + y_slope / STEEPNESS_FACTOR);

    if (s_cursor_position.x < 0) {
        s_cursor_position.x = 0;
    } else if (s_cursor_position.x > SCREEN_W) {
        s_cursor_position.x = SCREEN_W;
    }

    if (s_cursor_position.y < 0) {
        s_cursor_position.y = 0;
    } else if (s_cursor_position.y > KEYPAD_HEIGHT) {
        s_cursor_position.y = KEYPAD_HEIGHT;
    }

    layer_mark_dirty(s_cursor_layer);
}

static void init() {
    // Create main Window
    s_main_window = window_create();
    window_set_background_color(s_main_window, COLOR_BG);
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

    light_enable(true);
}

static void deinit() {
    // Destroy main Window
    window_destroy(s_main_window);

    accel_data_service_unsubscribe();

    light_enable(false);
}

/** @} */

int main(void) {
    init();
    app_event_loop();
    deinit();
}
