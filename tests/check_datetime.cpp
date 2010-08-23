// checks that we don't get twice the same ISO datetime, even when calling it in a loop.
#include <iostream>
//#include "boost/date_time/gregorian/gregorian.hpp"
//#include "boost/date_time/posix_time/posix_time.hpp"
#include <string>
#include <cassert>
#include "timer.h"
#include "timing.h"

//#include <unistd.h> // usleep and sleep

#define VERBOSE false

void check_iso_string()
{
    std::string current;
    std::string previous = "";
    if (VERBOSE)
        std::cout << "ISO datetime string" <<std::endl;
    for (int i = 0; i < 10; i++)
    {
        current = timing::get_iso_datetime_for_now();
        if (VERBOSE)
            std::cout << "Now = " << current << std::endl; 
        assert(current != previous);
        previous = current;
    }
}

void check_long()
{
    long current;
    long previous = 0L;

    if (VERBOSE)
        std::cout << "long timestamp" <<std::endl;
    for (int i = 0; i < 10; i++)
    {
        current = timing::get_timestamp_now();
        if (VERBOSE)
            std::cout << "Now = " << current << std::endl; 
        assert(current != previous);
        previous = current;
    }
}
// checks the Timer class
void check_timer()
{
    float elapsed; // elapsed
    Timer timer = Timer();
    float previous = 0.0f;
    if (VERBOSE)
        std::cout << "Timer with floats" <<std::endl;
    for (int i = 0; i < 10; i++)
    {
        timer.tick();
        elapsed = timer.get_elapsed();
        if (VERBOSE)
            std::cout << "elapsed = " << elapsed << std::endl; 
        //if (previous >= 0.001f)
        //{
        //    assert(elapsed - previous >= 0.001f);
        //    assert(elapsed - previous <= 0.0005f);
        //}
        previous = elapsed;
        usleep(1000); // 1 ms
    }
}

int main(int argc, char *argv[]) 
{
    check_iso_string();
    check_long();
    check_timer();
    return 0;
}

