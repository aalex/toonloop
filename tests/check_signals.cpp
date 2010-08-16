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

void print_sum(float x, float y)
{
    std::cout << x << " + " << y << " = " << x + y << std::endl;
}

void test_a()
{

    HelloSlot slot;
    boost::signal<void ()> sig_a;
    sig_a.connect(slot);
    std::cout << "Calling the signal A." << std::endl;
    sig_a();
}

void test_b()
{

    boost::signal<void (float, float)> sig_b;
    sig_b.connect(&print_sum);
    std::cout << "Calling the signal B." << std::endl;
    sig_b(3.14159, 1.618);
}
int main(int argc, char **argv)
{
    test_a();
    test_b();
    return 0;
}

