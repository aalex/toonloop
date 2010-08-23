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
#include "moviesaver.h"
#include "saverworker.h"

MovieSaver::MovieSaver() : // const Clip &clip?
    is_busy_(false)
    //worker_(this)
{
}

/**
 * Starts saving a clip to a movie file.
 *
 * Also checks if busy and join thread if done.
 */
// TODO: store tasks and defer them to later.
bool MovieSaver::add_saving_task(Clip* clip)
{
    if (is_busy()) {
        std::cout << "The MovieSaver is busy !" << std::endl;
        return false; // failed adding task
    } // else:
    is_busy_ = true;
    // TODO: delete current_task_ if already allocated
    //delete &current_task_;
    //current_task_ = std::tr1::shared_ptr<SavingTask>(new SavingTask());
    //// delete current worker if there is one
    //delete &worker_;
    //worker_ = new SaverWorker(this); // create a brand new worker...

    // gather info about this clip:
    current_task_->clip_id_ = clip->get_id();
    std::cout << "MovieSaver Clip ID is " << current_task_->clip_id_ << "and size is " << clip->size() << std::endl;  

    // load image names
    current_task_->image_paths_.clear();
    
    //clip.lock_mutex(); // FIXME: do we need mutexes at all?
    //TODO: save in reverse order or ping pong - as well
    for (int i = 0; i < clip->size(); i++)
    {
        // TODO: store the SavingTaskInfo in a struct
        // Will containt the image_paths, file_extension and format, plus the path to the image directory, etc.
        current_task_->image_paths_.push_back(clip->get_image_full_path(&(clip->get_image(i))));
    }
    //clip.unlock_mutex();
    // See saverworker.h
    //will call worker_.operator()() in a thread 
    worker_thread_ = boost::thread(*worker_);
    return true;
}

/**
 * Called at startup to set output directory for clips.
 */
void MovieSaver::set_result_directory(std::string path) 
{
    result_directory_ = path;
}

/**
 * Returns if it's currently busy or not
 * Checks if the saving thread is done.
 */
//TODO: call a signal when done
bool MovieSaver::is_busy()
{
    if (is_busy_) {
        std::cout << "The MovieSaver is busy saving !" << std::endl;
        bool joined = false;
        boost::posix_time::millisec wait_time(0);
        joined = worker_thread_.timed_join(wait_time);
        if (joined)
        {
            std::cout << "The MovieSaver thread joined" << std::endl;
            is_busy_ = false;
            return false;
        } else {
            std::cout << "Did not join" << std::endl;
            return true;
        }
    } else {
        return false;
    }
}

SavingTask& MovieSaver::get_current_task() 
{
    return *current_task_;
}

