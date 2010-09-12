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
namespace statesaving
{
    const char *ROOT_NODE = "project";
    const char *PROJECT_NAME_PROPERTY = "name";
    const char *FILE_NAME = "project.xml";
    const char *CLIPS_NODE = "clips";
    const char *CLIP_NODE = "clip";
    const char *CLIP_ID_PROPERTY = "id";
    const char *IMAGES_NODE = "images";
    const char *IMAGE_NODE = "image";
    void write_project_file();
};

