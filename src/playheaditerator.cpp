/*
 * Toonloop
 *
 * Copyright (c) 2010 Alexandre Quessy <alexandre@quessy.net>
 * Copyright (c) 2010 Tristan Matthews <le.businessman@gmail.com>
 *
 * Toonloop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toonloop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the gnu general public license
 * along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "playheaditerator.h"
#include "unused.h"
#include <glib.h>

// base class
const std::string &PlayheadIterator::get_name()
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

// forward
const std::string ForwardIterator::name_ = "forward";

unsigned int ForwardIterator::do_iterate(unsigned int current, unsigned int length)
{
    if (current >= length - 1)
        return 0;
    else 
        return current + 1;
}

// backward
const std::string BackwardIterator::name_ = "backward";

unsigned int BackwardIterator::do_iterate(unsigned int current, unsigned int length)
{
    if (current <= 0)
        return length - 1;
    else 
        return current - 1;
}

// yoyo
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

// random
const std::string RandomIterator::name_ = "random";

unsigned int RandomIterator::do_iterate(unsigned int current, unsigned int length)
{
    UNUSED(current);
    return (unsigned int) g_random_int_range(0, length - 1);
}

// drunk
const std::string DrunkIterator::name_ = "drunk";

unsigned int DrunkIterator::do_iterate(unsigned int current, unsigned int length)
{
    // TODO: make drunk steps configurable
    gint32 difference = 4;
    current += (unsigned int) g_random_int_range(-difference, difference);
    current %= length;
    return current;
}

