// g++ -Wall -lboost_signals-mt -o check_signals check_signals.cpp
// We will eventually switch to boost::signals2, but for now, 
// Let's stick to the one that is packaged in Debian. 
#include <iostream>
#include <boost/signals2.hpp>

#define verbose false

class HelloSlot
{
    public:
        void operator()() const
        {
            if (verbose)
                std::cout << "Hello" << std::endl;
        }
};

void print_sum(float x, float y)
{
    if (verbose)
        std::cout << x << " + " << y << " = " << x + y << std::endl;
}

class DummyObject 
{
    public:
        void foo(int i);
};
void DummyObject::foo(int i)
{
    if (verbose)
        std::cout << "foo(" << i <<  ");" << std::endl;
}
/**
 * Simple example with no argument
 */
void test_a()
{

    HelloSlot slot;
    boost::signals2::signal<void ()> sig_a;
    sig_a.connect(slot);
    if (verbose)
        std::cout << "Calling the signal A." << std::endl;
    sig_a();
}
/**
 * Passing float arguments
 */
void test_b()
{

    boost::signals2::signal<void (float, float)> sig_b;
    sig_b.connect(&print_sum);
    if (verbose)
        std::cout << "Calling the signal B." << std::endl;
    sig_b(3.14159, 1.618);
}


///** 
// * Using an object's method.
// */
//void test_c()
//{
//    DummyObject obj;
//    boost::signal<void (int)> sig_c;
//    sig_c.connect(&obj.foo);
//    std::cout << "Calling the signal C." << std::endl;
//    sig_c(32768);
//}

int main(int argc, char **argv)
{
    test_a();
    test_b();
//    test_c();
    return 0;
}

