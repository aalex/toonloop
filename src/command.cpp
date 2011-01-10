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
#include "command.h"
#include "controller.h"
#include "unused.h"

// generic methods
const std::string Command::default_name_ = "default";

const std::string &Command::get_name() const
{
    return do_get_name();
}

void Command::apply(Controller &controller)
{
    do_apply(controller);
}

void Command::do_apply(Controller &controller)
{
    UNUSED(controller);
}

// add_image
const std::string AddImageCommand::name_ = "add_image";

void AddImageCommand::do_apply(Controller &controller)
{
    controller.add_frame();
}

// remove_image
const std::string RemoveImageCommand::name_ = "remove_image";

void RemoveImageCommand::do_apply(Controller &controller)
{
    controller.remove_frame();
}

// select_clip(i: clip_number)
const std::string SelectClipCommand::name_ = "select_clip";

void SelectClipCommand::do_apply(Controller &controller)
{
    controller.choose_clip(clip_number_);
}

// quit
const std::string QuitCommand::name_ = "quit";

void QuitCommand::do_apply(Controller &controller)
{
    controller.quit();
}

// video_record_on
const std::string VideoRecordOnCommand::name_ = "video_record_on";

void VideoRecordOnCommand::do_apply(Controller &controller)
{
    controller.enable_video_grabbing(true);
}

// video_record_off
const std::string VideoRecordOffCommand::name_ = "video_record_off";

void VideoRecordOffCommand::do_apply(Controller &controller)
{
    controller.enable_video_grabbing(false);
}

// set_float(s: name, f: value)
const std::string SetFloatCommand::name_ = "set_float";

void SetFloatCommand::do_apply(Controller &controller)
{
    controller.set_float_value(property_name_, property_value_);
}

// set_int(s: name, i: value)
const std::string SetIntCommand::name_ = "set_int";

void SetIntCommand::do_apply(Controller &controller)
{
    controller.set_int_value(property_name_, property_value_);
}

// save_clip
const std::string SaveCurrentClipCommand::name_ = "save_current_clip";

void SaveCurrentClipCommand::do_apply(Controller &controller)
{
    controller.save_current_clip();
}
