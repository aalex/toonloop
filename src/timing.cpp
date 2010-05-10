/* 
 * Time-related utilities.
 */
#include "boost/date_time/posix_time/posix_time.hpp"
#include <string>
#include "timing.h"

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

