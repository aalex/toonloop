#include <iostream>
#include <assert.h>
#include <list>
/**
 * A clip is a list of frame numbers.
 * It can be played and edited.
 */
class Clip {
    private:
        /** List of int. (file names) */
        std::list<unsigned int> images_;
        /** Writehead range: [0, n] The next frame index to write. */
        std::list<unsigned int>::iterator writehead_;
        /** Playhead range: [0, n-1] The next frame index to play. */
        std::list<unsigned int>::iterator playhead_;
        enum PlayheadDirection {
            BACKWARD, 
            FORWARD, 
            BACK_AND_FORTH
        } direction_;
    public:
        Clip() 
        {
            images_ = std::list<unsigned int>();
            writehead_ = images_.begin();
            playhead_ = images_.begin();
            std::cout << "Clip created" << std::endl;
        }
        ~Clip() 
        {
            std::cout << "Clip destroyed" << std::endl;
        }
        /** Adds an image before writehead index. */
        void add_image()
        {
            unsigned int image_number = 999; // TODO: create allocator.
            std::cout << "Adding image " << image_number << std::endl; 
            images_.insert(writehead_, image_number);
            ++writehead_;
        }
        /** Removes an image at writehead index. */
        void remove_image()
        {
            if (! images_.empty())
            {
                images_.erase(writehead_);
                std::cout << "Removing image " << std::endl; 
                --writehead_;
            } else {
                std::cout << "Not enough images to pop one." << std::endl; 
            }
        }
        void print_writehead_position()
        {
            std::cout << "Writehead: " << *writehead_ << std::endl;
            std::cout << "NUmber of frames: " << int(images_.size()) << std::endl;
        }

        void clear()
        {
            images_.clear();
            //TODO: writehead_ ?
            //TODO: playhead_  ?
        }
        void writehead_next()
        {

        }
        void writehead_previous()
        {

        }
        void writehead_goto(unsigned int index)
        {

        }
        /** Go to last image */
        void writehead_end()
        {
            writehead_ = images_.end();
        }
        /** Go to first image */
        void writehead_beginning()
        {
            writehead_ = images_.begin();
        }
};

int main(int argc, char *argv[])
{
    Clip clip = Clip();
    clip.add_image();
    clip.print_writehead_position();
    clip.remove_image();
    clip.print_writehead_position();
    assert(1 == 1);
    return 0;
}

