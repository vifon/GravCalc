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

#include "config.h"

/* Do not include pebble.h when compiling the unittests. */
#ifndef __cplusplus
#   include <pebble.h>             /* for bool */
#endif

#include <limits.h>
#include <stdlib.h>

/** The underlying fixed point representation. */
typedef int fixed;

/** The scaling factor of the fixed point numbers. */
#define FIXED_SCALE 100

/** Maximum representable value. */
static const fixed FIXED_MAX = INT_MAX;

fixed fixed_add(fixed lhs, fixed rhs, bool* overflow);
fixed fixed_subt(fixed lhs, fixed rhs, bool* overflow);
fixed fixed_mult(fixed lhs, fixed rhs, bool* overflow);
fixed fixed_div(fixed lhs, fixed rhs);
char* fixed_repr(fixed fixed, char* buffer, size_t size);
fixed str_to_fixed(const char* str, bool* overflow);
int fixed_to_int(fixed n);
fixed int_to_fixed(int n);
fixed fixed_pow(fixed base, int exponent, bool* overflow);

#endif
