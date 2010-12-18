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

#include <string>
#include <iostream>
#include "action.h"
#include "controller.h"

// add_image
const std::string AddImageAction::name_ = "add_image";

void AddImageAction::do_apply(Controller &controller) const
{
    controller.add_frame();
}

// remove_image
const std::string RemoveImageAction::name_ = "remove_image";

void RemoveImageAction::do_apply(Controller &controller) const
{
    controller.remove_frame();
}

// select_clip(i: clip_number)
const std::string SelectClipAction::name_ = "select_clip";

void SelectClipAction::do_apply(Controller &controller) const
{
    controller.choose_clip(clip_number_);
}

