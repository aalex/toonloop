/* 
 * Simply prints the current date time in the ISO format, with microseconds.
 */
#include "boost/date_time/posix_time/posix_time.hpp"
#include <iostream>
#include <string>

/**
 * Converts the current local time to an ISO string in the form 
 * YYYYMMDDTHHMMSS.fffffffff  where T is the date-time separator
 */
std::string get_now_as_iso_string()
{
    using namespace boost::posix_time;
    //get the current time from the clock -- micro second resolution
    ptime now = microsec_clock::local_time(); // posix_time::ptime
    return to_iso_string(now);
}

int main() 
{
    for (int i = 0; i < 10; i++)
    {
        std::cout << "Now = " << get_now_as_iso_string() << std::endl; 
    }
    return 0;
}

