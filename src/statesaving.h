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

/**
 * Contains constants for state saving to XML file.
 */

// better name for a constant from libxml2
#define XMLSTR BAD_CAST

namespace statesaving
{
    const char *ROOT_NODE = "toonloop_project";
    const char *PROJECT_NAME_ATTR = "name";
    const char *DEFAULT_PROJECT_NAME = "default";
    const char *FILE_NAME = "project.xml";
    const char *CLIPS_NODE = "clips";
    const char *CLIP_NODE = "clip";
    const char *CLIP_ID_PROPERTY = "id";
    const char *IMAGES_NODE = "images";
    const char *IMAGE_NODE = "image";
    const char *IMAGE_NAME_ATTR = "name";
}

