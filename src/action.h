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
#ifndef __ACTION_H__
#define __ACTION_H__

#include <string>

class Controller;

/**
 * Action for the controller to do.
 * 
 * Abstract base class for all actions.
 */
class Action
{
    public:
        Action() {}
        const std::string &get_name() const;
        void apply(Controller &controller) const;
    private:
        virtual const std::string &do_get_name() const = 0;
        virtual void do_apply(Controller &controller) const = 0;
};

/**
 * Adds an image to the currently edited clip.
 */
class AddImageAction : public Action
{
    public:
        AddImageAction() : 
            Action()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller) const;
};

/**
 * Removes an image from the currently edited clip.
 */
class RemoveImageAction : public Action
{
    public:
        RemoveImageAction() : 
            Action()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller) const;
};

/**
 * Selects a clip to edit.
 */
class SelectClipAction : public Action
{
    public:
        SelectClipAction(unsigned int clip_number) : 
            Action(),
            clip_number_(clip_number)
        {}
//        void set_clip_number(unsigned int clip_number) { clip_number_ = clip_number; }
//        unsigned int get_clip_number() const { return clip_number_; }
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual void do_apply(Controller &controller) const;
        unsigned int clip_number_;
};
/*
            NOP,
            ADD_IMAGE,
            REMOVE_IMAGE,
            VIDEO_RECORD_ON,
            VIDEO_RECORD_OFF,
            SELECT_CLIP,
            QUIT,
            SET_FLOAT,
            SET_INT
            // TODO: implement the other messages:
            //SAVE_CLIP,
            //SAVE_PROJECT,
            //MOVE_WRITEHEAD_LEFT,
            //MOVE_WRITEHEAD_RIGHT,
            //MOVE_WRITEHEAD_BEGINNING,
            //MOVE_WRITEHEAD_END,
            //SET_PLAYHEAD_FPS,
*/
#endif
