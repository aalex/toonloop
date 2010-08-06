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

#include "moviesaver.h"
#include "subprocess.h"

MovieSaver::MovieSaver(Clip &clip) // const Clip &clip?
{
    // TODO: load image names
    clip_id_ = clip.get_id();
    std::cout << "Clip ID is " << clip_id_ << std::endl;  
}

void MovieSaver::operator()()
{
    std::string command = "sleep 1"; // TODO
    std::cout << "Lauching the command..." << std::endl;  
    bool ret_val = run_command(command); // blocking call
    std::cout << "MovieSaver: done saving clip" << std::endl;  
}

