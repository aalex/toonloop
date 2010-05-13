// g++ -I/usr/include -L/usr/lib -lboost_thread-mt -o t1 check_threads.cpp
#include <iostream>  
#include <boost/thread.hpp>  
#include <boost/date_time.hpp>  

class Worker
{
    public:
        Worker() {}

        /** 
         * Starts the thread. It's done when this method returns.
         */
        void operator()()
        {
            boost::posix_time::seconds workTime(1);  
            std::cout << "Worker: running during one second..." << std::endl;  
            boost::this_thread::sleep(workTime);  
            std::cout << "Worker: finished" << std::endl;  
        }
};

int main(int argc, char* argv[])  
{  
    std::cout << "main: startup" << std::endl;  
    Worker w = Worker();
    boost::thread workerThread(w);  

    boost::posix_time::millisec wait_time(0);
    boost::posix_time::millisec sleep_time(100);
    bool returned(false);
    while (! returned)
    {
        std::cout << "main: waiting for thread" << std::endl;  
        returned = workerThread.timed_join(wait_time);
        boost::this_thread::sleep(sleep_time); // simulates doing something else.
    }
    std::cout << "main: done" << std::endl;  
    return 0;  
}

