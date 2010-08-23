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
#include <boost/filesystem.hpp>
#include <iostream>
#include <glib.h>
#include "moviesaver.h"
#include "saverworker.h"
#include "subprocess.h" // TODO: use glib instead of subprocess.h
//#include "timing.h" // get_iso_dateime_for_now

SaverWorker::SaverWorker(MovieSaver *owner) :
    owner_(owner)
{
}

/**
 * Converts the series of images of a movie clip.
 */
void SaverWorker::operator()()
{
    // TODO: image paths and clip id be attribute of this
    int num_images = owner_->current_task_->image_paths_.size();
    int clip_id_ = owner_->current_task_->clip_id_;

    
    // TODO: create symlinks
    // TODO: get a handle to the clip 
    //
    // mencoder mf:///tmp/toonloop-LKJSD/*.jpg  -mf w=640:h=480:fps=2:type=jpg -ovc lavc -lavcopts vcodec=mjpeg -oac copy -of lavf -lavfopts format=mov -o out.mov
    // TODO: create a directory with get_iso_dateime_for_now or mkstemp 
    std::string command = "sleep 1"; // TODO
    std::cout << "Lauching $ " << command << std::endl;  
    bool ret_val = run_command(command); // blocking call
    //std::cout << "MovieSaver: done saving clip" << std::endl;  
    std::cout << "Done with $ " << command << std::endl;
    std::cout << "Its return value is " << ret_val << std::endl;
}

