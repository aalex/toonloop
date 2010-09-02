/*
 * Toonloop
 *
 * Copyright 2010 Alexandre Quessy
 * <alexandre@quessy.net>
 * http://www.toonloop.com
 *
 * Toonloop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toonloop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the gnu general public license
 * along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __TIMING_H__
#define __TIMING_H__

#include <string>

namespace timing 
{
    /** By how much divide timestamp obtained with get_timestamp_now in order to get seconds.
    */
    const long TIMESTAMP_PRECISION = 1000000L;
    std::string get_iso_datetime_for_now();
    long get_timestamp_now();
    //float get_time_now();
}

#endif // __TIMING_H__

