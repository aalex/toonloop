#include <iostream>
#include <assert.h>

class Test
{
    private:
        bool val_;
    public:
        Test();
        bool check();
};

Test::Test()
{
    val_ = false;
}

bool Test::check()
{
    if (val_ == false)
    {
        val_ = true;
        return true; // assigned it
    } else {
        return false; // was already assigned
    }
    return val_;
}

int main()
{
    Test t = Test();
    assert(t.check());
    assert(! t.check());
    //std::cout << "ok" << std::endl;
    return 0;
}

