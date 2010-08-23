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
#ifndef __SAVERWORKER_H__
#define __SAVERWORKER_H__

#include <boost/filesystem.hpp>
#include "clip.h"

// forward declarations
class MovieSaver;

class SaverWorker
{
    public:
        SaverWorker(MovieSaver *owner);
        void operator()();
        bool success_;
    private:
        MovieSaver *owner_;
};

#endif

