/* 
 * Time-related utilities.
 */
#include "boost/date_time/posix_time/posix_time.hpp"
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
    ptime time_t_epoch(date(1970, 1, 1)); 
    //std::cout << time_t_epoch << std::endl;
    // first convert nyc_time to utc via the utc_time() 
    // call and subtract the ptime.
    //time_duration diff = nyc_time.utc_time() - time_t_epoch;
    time_duration diff = now - time_t_epoch;
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


/** 
 * Float timer
 */
Timer::Timer()
{
    reset();
}
/**
 * Reset the start time of the timer to now.
 */
void Timer::reset()
{
    start_time_ = timing::get_timestamp_now();
    now_ = start_time_;
}
/**
 * Updates the current time.
 * Returns the current time.
 */
float Timer::tick()
{
    now_ = timing::get_timestamp_now();
    return now_;
}
/**
 * Returns how many seconds - with float precision - has elapsed since the creation of this object.
 * Note that you must call tick() prior to call this method if you want the current time.
 */
float Timer::get_elapsed()
{
    return (now_ - start_time_) * 0.000001;
}

