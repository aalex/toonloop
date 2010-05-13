// tests a list of object that can disappear anytime in a multithread world
#include <iostream>
#include <list>
#include <tr1/memory>

using namespace std::tr1; // shared_ptr

class Dummy
{
    public:
        int i_;
        Dummy(int value);
};

Dummy::Dummy(int value)
{
    i_ = value;
}

class Hello
{
    public:
        std::list< shared_ptr<Dummy> > dummies_;
        Hello();
};

Hello::Hello()
{
    for (int i = 0; i < 3; ++i)
    {
        dummies_.push_back(shared_ptr<Dummy>(new Dummy(i))); // appends
    }
}

int main(int argc, char **argv)
{
    Hello h = Hello();
    std::list< shared_ptr<Dummy> >::iterator iter;
    for (iter = h.dummies_.begin(); iter != h.dummies_.end(); iter++)
        std::cout << "Item:" << (*iter)->i_ << std::endl;
    return 0;
}

