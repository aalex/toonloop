#include <iostream>
#include <assert.h>
#include <list>

class Clip {
    private:
        std::list<int> images_;
    public:
        Clip() 
        {
            images_ = std::list<int>();
            std::cout << "Clip created" << std::endl;
        }
        ~Clip() 
        {
            std::cout << "Clip destroyed" << std::endl;
        }
        void add_image(int i)
        {
            std::cout << "Adding image " << i << std::endl; 
            images_.push_back(i); // append
        }
        void remove_image()
        {
            if (! images_.empty())
            {
                images_.pop_back(); // pop
                std::cout << "Removing image " << std::endl; 
            } else {
                std::cout << "Not enough images to pop one." << std::endl; 
            }
        }
};

int main(int argc, char *argv[])
{
    Clip clip = Clip();
    clip.add_image(1);
    clip.remove_image();
    assert(1 == 1);
    return 0;
}

