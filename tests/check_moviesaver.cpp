// tests that we can run a command in a thread.
// It also uses the clip and image classes from the app
#include <iostream>
#include <boost/date_time.hpp>  
#include "moviesaver.h"
#include "saverworker.h"
#include "clip.h"

int main(int argc, char* argv[])  
{  
    std::cout << "main: startup" << std::endl;  
    Clip clip(99);
    MovieSaver saver(clip);
    saver.start_saving();
    boost::posix_time::millisec sleep_time(100);
    bool done(false);
    while (! done)
    {
        std::cout << "main: waiting for thread" << std::endl;  
        done = saver.is_done();
        boost::this_thread::sleep(sleep_time); // simulates doing something else.
    }
    return 0;
}
