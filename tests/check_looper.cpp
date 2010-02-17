// tests the map and allocation.
#include <map>
#include <iostream>
#include <tr1/unordered_map>

namespace TestData 
{
    int created_ = 0;
    int destroyed_ = 0;
    void incrementDestroyed();
    void incrementCreated();
}

void TestData::incrementCreated()
{
    created_ += 1;
}

void TestData::incrementDestroyed()
{
    destroyed_ += 1;
}

class Clip {
    public:
        Clip() 
        {
            TestData::incrementCreated();
        }
        ~Clip() 
        {
            TestData::incrementDestroyed();
        }
};

int main(int argc, char *argv[])
{
    std::tr1::unordered_map<int, Clip*> dict;
    dict[1] = new Clip();
    dict[2] = new Clip();
    if (TestData::created_ != 2)
    {
        std::cout << "Num created:" << TestData::created_ << std::endl;
        return 1;
    }
    delete dict[1];
    delete dict[2];
    if (TestData::destroyed_ != 2)
    {
        std::cout << "Num destroyed:" << TestData::destroyed_ << std::endl;
        return 1;
    }
    return 0;
}

