// checks that we don't get twice the same ISO datetime, even when calling it in a loop.
#include <iostream>
#include <string>
#include <cassert>
#include "timing.h"

#define VERBOSE true

int main(int argc, char *argv[]) 
{
    std::string current;
    std::string previous = "";
    for (int i = 0; i < 10; i++)
    {
        current = timing::get_iso_datetime_for_now();
        if (VERBOSE)
            std::cout << "Now = " << current << std::endl; 
        assert(current != previous);
        previous = current;
    }
    return 0;
}

