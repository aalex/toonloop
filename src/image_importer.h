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

#ifndef __IMAGE_IMPORTER_H__
#define __IMAGE_IMPORTER_H__

#include <string>

/** 
 * Imports and resizes images.
 */
class ImageImporter 
{
    public: 
        ImageImporter(const std::string &input_file_name, const std::string &output_file_name, unsigned int width, unsigned int heigth, bool verbose);
        ~ImageImporter();
        bool resize();
    private:
        std::string input_file_name_;
        std::string output_file_name_;
        unsigned int width_;
        unsigned int height_;
        bool verbose_;
};

#endif // __IMAGE_IMPORTER_H__

