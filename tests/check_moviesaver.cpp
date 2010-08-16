// tests that we can run a command in a thread.
// It also uses the clip and image classes from the app
#include <iostream>
//#include <boost/thread.hpp>  
#include <boost/date_time.hpp>  
#include "moviesaver.h"
#include "clip.h"

int main(int argc, char* argv[])  
{  
    std::cout << "main: startup" << std::endl;  
    Clip clip(99);
    MovieSaver saver = MovieSaver(clip);
    //boost::thread workerThread(s);  

    //boost::posix_time::millisec wait_time(0);
    saver.start_saving(); // actually calling sleep 1
    boost::posix_time::millisec sleep_time(100);
    bool done(false);
    while (! done)
    {
        std::cout << "main: waiting for thread" << std::endl;  
        done = saver.is_done();
        //workerThread.timed_join(wait_time);
        boost::this_thread::sleep(sleep_time); // simulates doing something else.
    }
    return 0;
}
