/** @file fixed.h
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

#ifndef _h_FIXED_
#define _h_FIXED_

/** The underlying fixed point representation. */
typedef int fixed;

/** The scaling factor of the fixed point numbers. */
#define FIXED_SCALE 100

/** Sum two fixed point numbers.
 *
 *  @param lhs
 *  @param rhs
 *
 *  @return The result.
 */
fixed fixed_add(fixed lhs, fixed rhs)
{
    return lhs + rhs;
}

/** Subtract two fixed point numbers.
 *
 *  @param lhs
 *  @param rhs
 *
 *  @return The result.
 */
fixed fixed_subt(fixed lhs, fixed rhs)
{
    return lhs - rhs;
}

/** Multiply two fixed point numbers.
 *
 *  @param lhs
 *  @param rhs
 *
 *  @return The result.
 */
fixed fixed_mult(fixed lhs, fixed rhs)
{
    return (lhs * rhs) / FIXED_SCALE;
}

/** Divide two fixed point numbers.
 *
 *  @param lhs
 *  @param rhs
 *
 *  @return The result.
 */
fixed fixed_div(fixed lhs, fixed rhs)
{
    return (lhs * FIXED_SCALE) / rhs;
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
        snprintf(buffer, size,
                 "%s%d.%02u",
                 sign, integal_part, fractional_part);
    } else {
        snprintf(buffer, size,
                 "%s%d",
                 sign, integal_part);
    }

    return buffer;
}

#endif
