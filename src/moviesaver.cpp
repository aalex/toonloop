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

MovieSaver::MovieSaver() : // const Clip &clip?
    is_done_(false),
    is_saving_(false)
    //worker_(this)
{
}

/**
 * Starts saving a clip to a movie file.
 */
// TODO: store tasks and defer them to later.
bool MovieSaver::add_saving_task(Clip &clip)
{
    if (is_saving_) {
        std::cout << "The MovieSaver has already started saving!" << std::endl;
        return false;
    }
    is_saving_ = true;
    is_done_ = false;
    // TODO: delete current_task_ if already allocated
    current_task_ = std::tr1::shared_ptr<SavingTask>(new SavingTask());
    // TODO: delete current worker if there is one
    worker_ = new SaverWorker(this); // create a brand new worker...

    // gather info about this clip:
    current_task_->clip_id_ = clip.get_id();
    std::cout << "MovieSaver Clip ID is " << current_task_->clip_id_ << std::endl;  

    // load image names
    current_task_->image_paths_.clear();

    // TODO: create symlinks
    // TODO: get a handle to the clip 
    
    clip.lock_mutex(); // FIXME: do we need mutexes at all?
    //TODO: save in reverse order or ping pong - as well
    for (int i = 0; i < clip.size(); i++)
    {
        // TODO: store the SavingTaskInfo in a struct
        // Will containt the image_paths, file_extension and format, plus the path to the image directory, etc.
        current_task_->image_paths_.push_back(clip.get_image_full_path(&(clip.get_image(i))));
    }
    clip.unlock_mutex();
    //will call worker_.operator()() in a thread 
    worker_thread_ = boost::thread(*worker_);
    return true;
}


void MovieSaver::set_result_directory(std::string path) 
{
    result_directory_ = path;
}

/**
 * Returns if it's currently busy or not
 */
bool MovieSaver::is_saving()
{
    return is_saving_;
}
/**
 * Checks if the saving thread is done.
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

SavingTask& MovieSaver::get_current_task() 
{
    return *current_task_;
}

