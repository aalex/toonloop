// g++ -I/usr/include -L/usr/lib -lboost_thread-mt -o t1 check_threads.cpp
#include <iostream>  
#include <boost/thread.hpp>  
#include <boost/date_time.hpp>  
class Worker
{
    public:
        Worker() {}

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
    std::cout << "main: waiting for thread" << std::endl;  
    workerThread.join();  
    std::cout << "main: done" << std::endl;  
    return 0;  
}

