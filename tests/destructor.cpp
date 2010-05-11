#include <iostream>

class Dummy
{
    public:
        Dummy();
        ~Dummy();
    private:
        char *data_;
};

Dummy::Dummy()
{
    std::cout << "new" << std::endl;
    data_ = new char[100];
}
Dummy::~Dummy()
{
    std::cout << "delete" << std::endl;
    delete data_;
}


int main(int argc, char **argv)
{
    Dummy *d = new Dummy();
    delete d;
    return 0;
}

