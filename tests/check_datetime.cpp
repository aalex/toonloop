/* The following is a simple example that shows conversion of dates 
 * to and from a std::string.
 * 
 * Expected output:
 * 2001-Oct-09
 * 2001-10-09
 * Tuesday October 9, 2001
 * An expected exception is next: 
 * Exception: Month number is out of range 1..12
 */

#include "boost/date_time/gregorian/gregorian.hpp"
#include <iostream>
#include <string>

int main() 
{
  using namespace boost::gregorian;

    // Extended ISO format:
    //%Y-%m-%d %H:%M:%S%F%Q
    date d(2005, Jun, 25);
    date_facet* facet(new date_facet("%A %B %d, %Y"));
    //std::cout.imbue(std::locale(std::cout.getloc(), facet));
    std::cout << d << std::endl;
    // "Saturday June 25, 2005"
    //
#if 0
    try {


        // The following date is in ISO 8601 extended format (CCYY-MM-DD)
        std::string s("2001-10-9"); //2001-October-09
        date d(from_simple_string(s));
        std::cout << to_simple_string(d) << std::endl;


        //Read ISO Standard(CCYYMMDD) and output ISO Extended
        std::string ud("20011009"); //2001-Oct-09
        date d1(from_undelimited_string(ud));
        std::cout << to_iso_extended_string(d1) << std::endl;
        
        //Output the parts of the date - Tuesday October 9, 2001
        date::ymd_type ymd = d1.year_month_day();
        greg_weekday wd = d1.day_of_week();
        std::cout << wd.as_long_string() << " "
                  << ymd.month.as_long_string() << " "
                  << ymd.day << ", " << ymd.year
                  << std::endl;

        //Let's send in month 25 by accident and create an exception
        std::string bad_date("20012509"); //2001-??-09
        std::cout << "An expected exception is next: " << std::endl;
        date wont_construct(from_undelimited_string(bad_date));
        //use wont_construct so compiler doesn't complain, but you wont get here!
        std::cout << "oh oh, you shouldn't reach this line: " 
                  << to_iso_string(wont_construct) << std::endl;
    }
    catch(std::exception& e) {
        std::cout << "  Exception: " <<  e.what() << std::endl;
    }
#endif
    return 0;
}

# if 0 


// date and time formatting prototype.

using namespace boost::date_time;

int main(int argc, char *argv[])
{
    // Extended ISO format:
    //%Y-%m-%d %H:%M:%S%F%Q
    date d(2005,Jun,25);
    date_facet* facet(new date_facet("%A %B %d, %Y"));
    std::cout.imbue(std::locale(std::cout.getloc(), facet));
    std::cout << d << std::endl;
    // "Saturday June 25, 2005"
    return 0;
}
# endif
