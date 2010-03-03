#include <iostream>

class A {
    public:
        void hello() {
            _hello();
        };
    private:
        virtual void _hello() = 0; 
};

class B: public A {
    private:
        virtual void _hello() {
            std::cout << "B" << std::endl;
        }
};

void say_hello(A *a)
{
    a->hello();
}

int main()
{
    B b = B();
    say_hello(&b);
    return 0;
}
