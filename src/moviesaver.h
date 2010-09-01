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

#ifndef __MOVIESAVER_H__
#define __MOVIESAVER_H__

#include <boost/thread.hpp>  
#include <string>
#include <vector>
#include <tr1/memory>
#include "clip.h"
#include "saverworker.h"

/** We store there a snapshot of the info about a clip to save.
 */
struct SavingTask
{
    std::vector<std::string> image_paths_;
    int clip_id_;
    int fps_;
};
/** Saves one clip at a time to a movie in a thread.
 */
class MovieSaver
{
    public:
        MovieSaver();
        bool add_saving_task(Clip& clip);
        bool is_busy();
        void set_result_directory(const std::string &path);
        std::string get_result_directory() { return result_directory_; } 
        SavingTask &get_current_task() { return current_task_; };

        SavingTask current_task_;
        //void save(); // starts the thread
        // Starts the thread. It's done when this method returns.
        /**
         * list of absolute path to each images to save.
         */
    private:
        // let's store its ID
        //bool is_done_;
        //bool is_saving_;
        bool is_busy_;
        std::string result_directory_;
        SaverWorker worker_;
        boost::thread worker_thread_;
};

#endif

