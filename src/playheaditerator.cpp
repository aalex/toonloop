#include "playheaditerator.h"

const std::string &PlayheadIterator::get_name() const
{
    return do_get_name();
}

unsigned int PlayheadIterator::iterate(unsigned int current, unsigned int length)
{
    if (length <= 1)
        return 0;
    else
        return do_iterate(current, length) % length;
}

const std::string ForwardIterator::name_ = "forward";

unsigned int ForwardIterator::do_iterate(unsigned int current, unsigned int length)
{
    if (current >= length - 1)
        return 0;
    else 
        return current + 1;
}

const std::string BackwardIterator::name_ = "backward";

unsigned int BackwardIterator::do_iterate(unsigned int current, unsigned int length)
{
    if (current <= 0)
        return length - 1;
    else 
        return current - 1;
}

const std::string YoyoIterator::name_ = "yoyo";

unsigned int YoyoIterator::do_iterate(unsigned int current, unsigned int length)
{
    if (yoyo_direction_ == -1) // backward
    {
        if (current <= 0)
        {
            current = 1;
            yoyo_direction_ = 1; // change to forward
        } else 
            --current;
    } 
    else // forward
    {
        if (current >= length - 1)
        {
            current = length - 2; // at this point, we know length >= 2
            yoyo_direction_ = -1; // change to backward
        } 
        else 
            ++current;
    }
    return current;
}

