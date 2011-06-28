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
#include <boost/filesystem.hpp>
#include <iostream>
#include <glib.h>
#include <stdio.h>
#include "moviesaver.h"
#include "saverworker.h"
#include "subprocess.h" // TODO: use glib instead of subprocess.h

SaverWorker::SaverWorker(MovieSaver *owner) :
    owner_(owner)
{
    datetime_started_ = "";
    final_file_name_ = "";
}

void SaverWorker::set_final_options(const std::string &datetime_started, const std::string &final_file_name)
{
    datetime_started_ = datetime_started;
    final_file_name_ = final_file_name;
}

/**
 * Converts the series of images of a movie clip.
 */
void SaverWorker::operator()()
{
    namespace fs = boost::filesystem;
    success_ = false;
    // TODO: make FPS configurable
    std::ostringstream fps_os;
    fps_os << owner_->current_task_.fps_;
    std::string fps = fps_os.str();
    // TODO: image paths and clip id be attribute of this
    //int num_images = owner_->current_task_.image_paths_.size();
    //int clip_id = owner_->current_task_.clip_id_;
    //std::cout << "In the saving thread for clip #" << clip_id << " with " << num_images << " images" << std::endl; // TODO
    
    // TODO: create symlinks
    fs::path directory = fs::path(g_get_tmp_dir()) / fs::path("toonloop-" + datetime_started_);
    //std::cout << "----------------------------------" << std::endl;
    //std::cout << "tmp dir: " << directory.string() << std::endl;
    //std::cout << "----------------------------------" << std::endl;

    if (!fs::exists(directory)) 
    { 
        //std::cout << "creating directory " << directory.string() << std::endl;
        bool success = fs::create_directory(directory);
        if (!success)
        {
            std::cout << "failed to create directory" << std::endl;
            success_ = false;
            return;
        }
    } else {
        std::cout << "directory " << directory.string() << "already exists" << std::endl;
    }
    int BUFSIZE = 11;
    char buffer[BUFSIZE];
    for (unsigned i = 0; i < owner_->current_task_.image_paths_.size(); i++)
    {
        // TODO: store the SavingTaskInfo in a struct
        // Will containt the image_paths, file_extension and format, plus the path to the image directory, etc.
        snprintf(buffer, BUFSIZE, "%06d.jpg", i);
        fs::path to_p = fs::path(owner_->current_task_.image_paths_[i]);
        fs::path from_p = directory / fs::path(buffer);
        //std::cout << " $ ln -s " << to_p.string() << " " << from_p.string() << std::endl;

        if (!fs::exists(to_p)) 
        { 
            std::cout << "Target doesn't exist!" << std::endl;
        } else if (fs::exists(from_p)) {
            std::cout << "Symlink already exists!" << std::endl;
        } else {
            //int error_code = 
            try
            {
                //std::cout << "creating symlink" << std::endl;
                fs::create_symlink(to_p, from_p);
            } catch(const fs::filesystem_error &e) {
                std::cerr << "Error creating symlink: " << e.what() << std::endl;
                success_ = false;
                return;
            }
            //if (error_code != 0)
            //    std::cout << "ERROR!" << std::endl;
            //std::cout << "Success creating symlink" << std::endl;
        }
    }
    fs::path output_movie = directory / fs::path("out.mov");

    // TODO: Make width/height configurable
    // TODO: support AVI file.. 
    //  mencoder "mf://*.jpg" -mf fps=5 -o test.avi -ovc lavc -lavcopts vcodec=msmpeg4v2:vbitrate=800
    
    int clip_w = owner_->current_task_.width_;
    int clip_h = owner_->current_task_.height_;

    std::ostringstream command;
    command << "mencoder mf://" << directory.string() << "/*.jpg -quiet -mf w=" << clip_w << ":h=" << clip_h << ":fps=" << fps << ":type=jpg -ovc lavc -lavcopts vcodec=mjpeg -oac copy -of lavf -lavfopts format=mov -o " << output_movie.string();
    std::cout << "Lauching $ " << command.str() << std::endl;  
    bool ret_val = subprocess::run_command(command.str()); // blocking call
    //std::cout << "Done with $ " << command << std::endl;
    if (ret_val != 0)
        std::cout << "Mencoder's return value was " << ret_val << std::endl;
    else
        std::cout << "Success!" << std::endl;
    // rename movie file
    try 
    {
        fs::copy_file(fs::path(output_movie), fs::path(final_file_name_));
        // the old file will be deleted with the whole dir
    }
    catch(fs::filesystem_error e) 
    { 
        std::cerr << "Error renaming final movie file : " << e.what() << std::endl;
        success_ = false;
        return;
    }   
    try
    {
        //std::cout << "remove directory tree" << std::endl;
        fs::remove_all(directory);
    } catch(const fs::filesystem_error &e) {
        std::cerr << "Error removing directory tree: " << e.what() << std::endl;
        success_ = false;
        return;
    }
    std::cout << "Done creating movie " << final_file_name_ << std::endl;
    success_ = true;
    return;
}

