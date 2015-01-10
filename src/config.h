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

/** Size of the input buffer (@ref s_input_buffer). */
#define INPUT_BUFFER_SIZE 32

/** The number of samples used for the calibration at the startup */
#define CALIBRATION_SAMPLES 10

#endif
