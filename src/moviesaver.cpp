/*
 * Toonloop
 *
 * Copyright 2010 Alexandre Quessy
 * <alexandre@quessy.net>
 * http://www.toonloop.com
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
#include <iostream>
#include <boost/thread.hpp>  
#include <boost/date_time.hpp>  
//#include <glib.h>
#include "moviesaver.h"
#include "saverworker.h"

MovieSaver::MovieSaver(Clip &clip) : // const Clip &clip?
    is_done_(false),
    is_saving_(false),
    worker_(this)
{
    // TODO: load image names
    // TODO: create symlinks
    // TODO: get a handle to the clip 
    clip_id_ = clip.get_id();
    std::cout << "MovieSaver Clip ID is " << clip_id_ << std::endl;  
}

bool MovieSaver::start_saving()
{
    if (is_saving_) {
        std::cout << "The MovieSaver has already started saving!" << std::endl;
        assert(false); // TODO: raise an exception
    }
    is_saving_ = true;
    is_done_ = false;

    //will call worker_.operator()() in a thread 
    worker_thread_ = boost::thread(worker_);

    return true;
}

bool MovieSaver::is_saving()
{
    return is_saving_;
}
/**
 * Checks if the thread is done.
 */
//TODO: call a signal when done
bool MovieSaver::is_done()
{
    if (! is_saving_) {
        std::cout << "The MovieSaver has not started saving!" << std::endl;
        assert(false); // TODO: raise an exception
    } else if (is_done_) {
        return true;
    } else {
        boost::posix_time::millisec wait_time(0);
        bool returned(false);
        returned = worker_thread_.timed_join(wait_time);
        is_done_ = returned;
        return returned;
    }
}

