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

#include "image_importer.h"
#include <clutter/clutter.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <iostream>

ImageImporter::ImageImporter(
    const std::string &input_file_name, 
    const std::string &output_file_name, 
    unsigned int width, 
    unsigned int heigth,
    bool verbose) :
        input_file_name_(input_file_name),
        output_file_name_(output_file_name),
        width_(width),
        height_(heigth),
        verbose_(verbose)
{}

ImageImporter::~ImageImporter()
{
}

/**
 * Actually scales the input image and saves it as output image.
 */
bool ImageImporter::resize()
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    if (verbose_)
        g_print("Loading image %s at size of %dx%d\n", input_file_name_.c_str(), width_, height_);
    pixbuf = gdk_pixbuf_new_from_file_at_scale(input_file_name_.c_str(), width_, height_, FALSE, &error);
    if (! pixbuf)
    {
        g_error("Error loading image %s scaled_pixbuf: %s", input_file_name_.c_str(), error->message);
        g_error_free(error);
        error = NULL;
        g_object_unref(pixbuf);
        return false;
    }
    std::string img_type = "jpeg";
    gboolean success = gdk_pixbuf_save(pixbuf, output_file_name_.c_str(), img_type.c_str(), &error, NULL);
    g_object_unref(pixbuf);
    if (error)
    {
        g_error("Error saving image %s: %s", output_file_name_.c_str(), error->message);
        g_error_free(error);
        return false;
    }
    if (! success)
    {
        std::cerr << "Failed to save " << output_file_name_ << std::endl;
        return false;
    }
    return true;
}

