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


