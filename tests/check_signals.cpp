// g++ -Wall -lboost_signals -o check_signals check_signals.cpp
//
#include <iostream>
#include "boost/signals.hpp"

class HelloSlot
{
    public:
        void operator()() const
        {
            std::cout << "Hello" << std::endl;
        }
};

int main(int argc, char **argv)
{
    HelloSlot slot;
    boost::signal<void ()> sig;
    sig.connect(slot);
    std::cout << "Calling the signal." << std::endl;
    sig();
    return 0;
}

