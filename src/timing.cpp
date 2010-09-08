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
/* 
 * Time-related utilities.
 */
#include "boost/date_time/posix_time/posix_time.hpp"
#include <iostream>
#include <string>
#include "timing.h"
//#include <glib.h>
#include "log.h"

/**
 * Returns the current local time as an ISO string in the form 
 * YYYYMMDDTHHMMSS.fffffffff  where T is the date-time separator
 */
std::string timing::get_iso_datetime_for_now()
{
    using namespace boost::posix_time;
    //get the current time from the clock -- micro second resolution
    ptime now = microsec_clock::local_time(); // posix_time::ptime
    return to_iso_string(now);
}

/**
 * Returns the current time as a UNIX timestamp, with a microsecond precision.
 */
long timing::get_timestamp_now()
{
    using namespace boost::posix_time;
    using namespace boost::gregorian;
    ptime now = microsec_clock::local_time();
    //std::cout << "now = " << now.total_microseconds() << std::endl;
    ptime time_epoch(date(1970, 1, 1)); 
    //std::cout << "epoch = " << time_epoch.total_microseconds() << std::endl;
    //std::cout << time_t_epoch << std::endl;
    // first convert nyc_time to utc via the utc_time() 
    // call and subtract the ptime.
    //time_duration diff = nyc_time.utc_time() - time_t_epoch;
    time_duration diff = now - time_epoch;
    return diff.total_microseconds();
}

/**
 * Returns the current time in seconds, float precision.
 * Uses Glib's time style. 
 */
// // FIXME: is this a timestamp or what?
// float timing::get_time_now()
// {
//     float ret;
//     GTimeVal current_time;
//     g_get_current_time(&current_time);
//     LOG_INFO("Seconds:" << current_time.tv_sec << " usec:" << current_time.tv_usec); 
// 
//     ret = current_time.tv_sec + (current_time.tv_usec * 0.000001f);
//     // FIXME: does this cause memory leak?
//     return ret;
// }

