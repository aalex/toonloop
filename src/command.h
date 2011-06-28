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
#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <string>

class Controller;

/**
 * Action for the controller to do. It's an implementation of the command design pattern.
 * 
 * Abstract base class for all commands.
 */
class Command
{
    public:
        Command() {}
        const std::string &get_name() const;
        void apply(Controller &controller);
        virtual bool is_reversible() { return false; }
        virtual void undo() { }
        virtual ~Command() {}
    private:
        static const std::string default_name_;
        virtual const std::string &do_get_name() const { return default_name_; }
        virtual void do_apply(Controller &controller);
};

/**
 * Adds an image to the currently edited clip.
 */
class AddImageCommand : public Command
{
    public:
        AddImageCommand() : 
            Command()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
};

/**
 * Removes an image from the currently edited clip.
 */
class RemoveImageCommand : public Command
{
    public:
        RemoveImageCommand() : 
            Command()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
};

/**
 * Selects a clip to edit.
 */
class SelectClipCommand : public Command
{
    public:
        SelectClipCommand(unsigned int clip_number) : 
            Command(),
            clip_number_(clip_number)
        {}
//        void set_clip_number(unsigned int clip_number) { clip_number_ = clip_number; }
//        unsigned int get_clip_number() const { return clip_number_; }
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
        unsigned int clip_number_;
};


/**
 * Quits the application.
 */
class QuitCommand : public Command
{
    public:
        QuitCommand() : 
            Command()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
};


/**
 * Starts video grabbing.
 */
class VideoRecordOnCommand : public Command
{
    public:
        VideoRecordOnCommand() : 
            Command()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
};

/**
 * Stops video grabbing.
 */
class VideoRecordOffCommand : public Command
{
    public:
        VideoRecordOffCommand() : 
            Command()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
};

/**
 * Sets the value of a float property.
 */
class SetFloatCommand : public Command
{
    public:
        SetFloatCommand(const std::string name, float value) : 
            Command(),
            property_name_(name),
            property_value_(value)
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
        std::string property_name_;
        float property_value_;
};

/**
 * Sets the value of an int property.
 */
class SetIntCommand : public Command
{
    public:
        SetIntCommand(const std::string name, int value) : 
            Command(),
            property_name_(name),
            property_value_(value)
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
        std::string property_name_;
        int property_value_;
};

/**
 * Saves the currently selected clip.
 */
class SaveCurrentClipCommand : public Command
{
    public:
        SaveCurrentClipCommand() : 
            Command()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
};

/*
            // TODO: implement the other messages:
            //SAVE_CLIP,
            //SAVE_PROJECT,
            //MOVE_WRITEHEAD_LEFT,
            //MOVE_WRITEHEAD_RIGHT,
            //MOVE_WRITEHEAD_BEGINNING,
            //MOVE_WRITEHEAD_END,
            //SET_PLAYHEAD_FPS,
*/

/**
 * Imports an image to the current clip.
 */
class ImportImageCommand : public Command
{
    public:
        ImportImageCommand(const std::string file_name) : 
            Command(),
            file_name_(file_name)
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
        std::string file_name_;
};

/**
 * Sets the loop bounds for the current clip
 */
class LoopBoundsCommand : public Command
{
    public:
        LoopBoundsCommand(double lower, double upper) : 
            Command(),
            lower_(lower),
            upper_(upper)
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller);
        double lower_;
        double upper_;
};
#endif
