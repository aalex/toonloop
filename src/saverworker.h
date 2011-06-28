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

#ifndef __SAVERWORKER_H__
#define __SAVERWORKER_H__

// forward declarations
class MovieSaver;

/** Takes care of saving a movie in a thread.
 */
class SaverWorker
{
    public:
        SaverWorker(MovieSaver *owner);
        void set_final_options(const std::string &datetime_started, const std::string &final_file_name);
        void operator()();
        bool success_;
    private:
        MovieSaver *owner_;
        std::string datetime_started_;
        std::string final_file_name_;
};

#endif

