// g++ -Wall -lboost_signals -o check_signals check_signals.cpp
//
#include <iostream>
#include "boost/signals.hpp"

class Hello
{
    public:
        void operator()() const
        {
            std::cout << "Hello" << std::endl;
        }
};

int main(int argc, char **argv)
{
    boost::signal<void ()> sig;
    Hello hello;
    sig.connect(hello);
    sig();
    return 0;
}

