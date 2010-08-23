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
    Clip clip(99);// dummy id, no images.
    MovieSaver* saver = new MovieSaver();
    saver->add_saving_task(&clip);
    boost::posix_time::millisec sleep_time(100);
    while (saver->is_busy())
    {
        std::cout << "main: waiting for thread" << std::endl;  
        boost::this_thread::sleep(sleep_time); // simulates doing something else.
    }
    return 0;
}
