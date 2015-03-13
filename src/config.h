/** @file config.h */
#ifndef _h_CONFIG_
#define _h_CONFIG_

/** Performing an operation automatically pushes the result to the
 *  stack instead of storing it in the editing buffer.
 */
#define ENABLE_AUTOPUSH 1



/** Size of the calculator stack (@ref s_calculator_stack). */
#define CALC_STACK_SIZE 64
/** Numeric type used for the calculations. */
#define CALC_TYPE fixed
/** @p printf format specifier for @ref CALC_TYPE. */
#define CALC_TYPE_FMT "%s"

/** Concatenate two symbols with an underline in-between.
 *
 *  @note This macro is necessary to force the macro expansion of the
 *  arguments by the caller, as the symbols/arguments used in the ##
 *  operator are not expanded.
 */
#define CONCAT_SYMBOLS(P1, P2) P1##_##P2

/** The same as @ref CREATE_OPERATOR but for any type. Needed to force
 *  the macro expansion of CALC_TYPE.
 */
#define CREATE_OPERATOR_FOR_TYPE(TYPE, OP) CONCAT_SYMBOLS(TYPE, OP)

/** Save the name of the function providing the operator for the @ref
 *  CALC_TYPE by concatenating CALC_TYPE with an underscore and one
 *  of:
 *
 *  - add,
 *  - subt,
 *  - mult,
 *  - div,
 *  - pow,
 *  - repr.
 */
#define CREATE_OPERATOR(OP) CREATE_OPERATOR_FOR_TYPE(CALC_TYPE, OP)

#define ADD CREATE_OPERATOR(add)
#define SUBT CREATE_OPERATOR(subt)
#define MULT CREATE_OPERATOR(mult)
#define DIV CREATE_OPERATOR(div)
#define POW CREATE_OPERATOR(pow)
#define REPR CREATE_OPERATOR(repr)



/** Size of the input buffer (@ref s_input_buffer). */
#define INPUT_BUFFER_SIZE 32

/** The number of samples used for the calibration at the startup */
#define CALIBRATION_SAMPLES 10


// #ifdef PBL_COLOR

// #   define COLOR_DISPLAY_TEXT TODO
// #   define COLOR_DISPLAY_BG TODO

// #   define COLOR_BUTTON_TEXT TODO
// #   define COLOR_BUTTON_BG TODO
// #   define COLOR_BUTTON_BORDER TODO

// #   define COLOR_BUTTON_FOCUSED_TEXT TODO
// #   define COLOR_BUTTON_FOCUSED_BG TODO
// #   define COLOR_BUTTON_FOCUSED_BORDER TODO

// #   define COLOR_BG TODO

// #   define COLOR_CURSOR TODO
// #   define COLOR_CURSOR_BORDER TODO

// #else

#   define COLOR_DISPLAY_TEXT GColorBlack
#   define COLOR_DISPLAY_BG GColorWhite

#   define COLOR_BUTTON_TEXT GColorWhite
#   define COLOR_BUTTON_BG GColorBlack
#   define COLOR_BUTTON_BORDER GColorWhite

#   define COLOR_BUTTON_FOCUSED_TEXT GColorBlack
#   define COLOR_BUTTON_FOCUSED_BG GColorWhite
#   define COLOR_BUTTON_FOCUSED_BORDER GColorWhite

#   define COLOR_BG GColorBlack

#   define COLOR_CURSOR GColorWhite
#   define COLOR_CURSOR_BORDER GColorBlack

// #endif

#endif
