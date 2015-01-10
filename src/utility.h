/** @file utility.h
 *  @brief Various utility functions.
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

#ifndef _h_UTILITY_
#define _h_UTILITY_

#include "config.h"

/** Convert a string to integer.
 *
 *  @param str String to convert.
 *  @param[out] endptr If non-NULL, set to the first invalid character.
 *  @param maxnum Max number of digits to read. Pass -1 for unlimited.
 *
 *  @return The converted integer.
 */
int str_to_int(const char *str, char **endptr, int maxnum) {
    int result = 0;

    int sign = 1;
    if (*str == '-') {
        ++str;
        sign = -1;
    }

    while (*str >= '0' && *str <= '9' && maxnum-- != 0) {
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

#endif
