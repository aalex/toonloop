/* 
 * Simply prints the current date time in the ISO format, with microseconds.
 */
#include "boost/date_time/posix_time/posix_time.hpp"
#include <iostream>
#include <string>
#include <cassert>

#define VERBOSE 0

/**
 * Converts the current local time to an ISO string in the form 
 * YYYYMMDDTHHMMSS.fffffffff  where T is the date-time separator
 */
std::string get_iso_datetime_for_now()
{
    using namespace boost::posix_time;
    //get the current time from the clock -- micro second resolution
    ptime now = microsec_clock::local_time(); // posix_time::ptime
    return to_iso_string(now);
}

int main(int argc, char *argv[]) 
{
    std::string current;
    std::string previous = "";
    for (int i = 0; i < 10; i++)
    {
        current = get_iso_datetime_for_now();
        if (VERBOSE)
            std::cout << "Now = " << current << std::endl; 
        assert(current != previous);
        previous = current;
    }
    return 0;
}

