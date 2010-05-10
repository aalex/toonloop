// checks that we don't get twice the same ISO datetime, even when calling it in a loop.
#include <iostream>
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <string>
#include <cassert>
#include "timing.h"

#define VERBOSE true

/**
 * Returns the current time as a UNIX timestamp, with a microsecond precision.
 */
long get_timestamp_now()
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

int main(int argc, char *argv[]) 
{
    std::string current;
    std::string previous = "";
    long current_long;
    long previous_long = 0L;

    for (int i = 0; i < 10; i++)
    {
        current = timing::get_iso_datetime_for_now();
        if (VERBOSE)
            std::cout << "Now = " << current << std::endl; 
        assert(current != previous);
        previous = current;
    }
    for (int i = 0; i < 10; i++)
    {
        current_long = get_timestamp_now();
        if (VERBOSE)
            std::cout << "Now = " << current_long << std::endl; 
        assert(current_long != previous_long);
        previous_long = current_long;
    }
    return 0;
}

