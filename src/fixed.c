/** @file fixed.c
 *  @brief A fixed point numbers implementation.
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

#include "fixed.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utility.h"

/** Sum two fixed point numbers.
 *
 *  @param lhs
 *  @param rhs
 *  @param[out] overflow Indicate whether the addition would
 *  result in an overflow. If the initial value is @p true, it will
 *  stay @p true. The returned value is unspecified if it is true.
 *
 *  @return The result.
 */
fixed fixed_add(fixed lhs, fixed rhs, bool* overflow)
{
    // If both arguments have the same sign...
    if ((rhs > 0) == (lhs > 0)) {
        // ...check for the overflow.
        *overflow = *overflow || abs(lhs) > FIXED_MAX - abs(rhs);
    }

    if (*overflow) {
        return lhs;
    }

    return lhs + rhs;
}

/** Subtract two fixed point numbers.
 *
 *  @param lhs
 *  @param rhs
 *  @param[out] overflow Indicate whether the subtraction would
 *  result in an overflow. If the initial value is @p true, it will
 *  stay @p true. The returned value is unspecified if it is true.
 *
 *  @return The result.
 */
fixed fixed_subt(fixed lhs, fixed rhs, bool* overflow)
{
    return fixed_add(lhs, -rhs, overflow);
}

/** Multiply two fixed point numbers.
 *
 *  @param lhs
 *  @param rhs
 *  @param[out] overflow Indicate whether the multiplication would
 *  result in an overflow. If the initial value is @p true, it will
 *  stay @p true. The returned value is unspecified if it is true.
 *
 *  @return The result.
 */
fixed fixed_mult(fixed lhs, fixed rhs, bool* overflow)
{
    if (fixed_to_int(rhs) != 0) {
        *overflow = *overflow || abs(lhs) > abs(fixed_div(FIXED_MAX, rhs));
    }

    if (*overflow) {
        return lhs;
    }

    return (lhs / FIXED_SCALE) * rhs
        + ((lhs % FIXED_SCALE) * rhs) / FIXED_SCALE;
}

/** Divide two fixed point numbers.
 *
 *  @param lhs
 *  @param rhs
 *
 *  @note The fractional part of @p rhs is ignored for large @p lhs
 *  due to a change in the order of performed operations made to avoid
 *  overflows.
 *
 *  @return The result.
 */
fixed fixed_div(fixed lhs, fixed rhs)
{
    /* Check if it's safe to normalize lhs instead of rhs for precision. */
    if (lhs < FIXED_MAX / FIXED_SCALE) {
        /* Keep precision whether possible. */
        lhs = lhs * FIXED_SCALE;
    } else {
        /* Sacrifice the fractional part of rhs when the another order
         * of operations would cause an overflow. */
        rhs = fixed_to_int(rhs);
    }

    if (rhs == 0) {
        return 0;               /* TODO: handle properly */
    } else {
        return lhs / rhs;
    }
}

/** Create the textual representation of the fixed point number.
 *
 *  @param fixed A number to represent.
 *  @param buffer A buffer to store the representation.
 *  @param size Size of @p buffer.
 *
 *  @return A pointer to the @p buffer parameter.
 */
char* fixed_repr(fixed fixed, char* buffer, size_t size)
{
    const char* sign = (fixed < 0 ? "-" : "");
    int integal_part = abs(fixed) / FIXED_SCALE;
    int fractional_part = abs(fixed) % FIXED_SCALE;

    if (fractional_part != 0) {
        int n = snprintf(buffer, size,
                         "%s%d.%02u",
                         sign, integal_part, fractional_part);

        /* Remove the trailing zeros. */
        if (fractional_part >= 10 &&
            fractional_part % 10 == 0) {

            buffer[n-1] = '\0';
        }
    } else {
        snprintf(buffer, size,
                 "%s%d",
                 sign, integal_part);
    }

    return buffer;
}

/** Convert a string to a fixed point number.
 *
 *  @param str String to convert.
 *  @param[out] overflow Indicate whether the conversion would
 *  result in an overflow. If the initial value is @p true, it will
 *  stay @p true. The returned value is unspecified if it is true.
 *
 *  @return The converted fixed point number.
 */
fixed str_to_fixed(const char* str, bool* overflow)
{
    char* fractional_start;
    char* endptr;

    int sign = 1;
    if (*str == '-') {
        ++str;
        sign = -1;
    }

    // Detect a potential overflow. The 8-digit numbers below
    // FIXED_MAX are currently wrongly detected too.
    static const int FIXED_MAX_digits = 8;
    const char* integral_end = strchr(str, '.');
    if (integral_end == NULL) {
        integral_end = str + strlen(str);
    }
    if (integral_end - str >= FIXED_MAX_digits) {
        *overflow = true;
        return 0;
    }

    int integral_part = str_to_int(str, &fractional_start, -1) * FIXED_SCALE;
    if (*fractional_start != '\0') {
        ++fractional_start;
    }
    int fractional_part = str_to_int(fractional_start, &endptr, 2);

    if (endptr - fractional_start == 1) {
        /* There was only one digit -- higher order of magnitude. */
        fractional_part *= 10;
    }

    return sign * (integral_part + fractional_part);
}

/** Convert the fixed point value to a regular integer.
 *
 *  @param n
 *
 *  @return The integral part of the fixed point number.
 */
int fixed_to_int(fixed n)
{
    return n / FIXED_SCALE;
}

/** Convert the integer to a fixed point value.
 *
 *  @param n
 *
 *  @return The converted fixed point number.
 */
fixed int_to_fixed(int n)
{
    return n * FIXED_SCALE;
}

/** A simple implementation of the <tt>pow(3)</tt> standard function
 *  for fixed point numbers.
 *
 *  @param base
 *  @param exponent
 *  @param[out] overflow Indicate whether the exponentiation would
 *  result in an overflow. If the initial value is @p true, it will
 *  stay @p true. The returned value is unspecified if it is true.
 *
 *  @return The exponentiation result.
 *
 *  @note The exponent must not be negative!
 */
fixed fixed_pow(fixed base, int exponent, bool* overflow)
{
    fixed result = FIXED_SCALE;

    bool negative = exponent < 0;
    exponent = abs(exponent);

    while (exponent-- && *overflow == false) {
        result = fixed_mult(result, base, overflow);
    }

    if (negative) {
        return fixed_div(int_to_fixed(1),
                         result);
    } else {
        return result;
    }
}
