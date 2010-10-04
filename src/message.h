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
#ifndef _MESSAGE_H_
#define _MESSAGE_H_
/**
 * Control message that is custom for Toonloop.
 */
class Message 
{
    public:
        enum Command 
        {
            ADD_IMAGE,
            REMOVE_IMAGE,
            VIDEO_RECORD_ON,
            VIDEO_RECORD_OFF,
            SELECT_CLIP
            // TODO: implement the other messages:
            //SAVE_CLIP,
            //SAVE_PROJECT,
            //MOVE_WRITEHEAD_LEFT,
            //MOVE_WRITEHEAD_RIGHT,
            //MOVE_WRITEHEAD_BEGINNING,
            //MOVE_WRITEHEAD_END,
            //SET_PLAYHEAD_FPS,
        };
        // Constructor with no arg
        Message() : 
            command_(), value_(0) {}
        /**
         * Constructor. One must provide at least a command id.
         */
        Message(Command c) : 
            command_(c), value_(0) {}
        /**
         * Constructor for commands which accept a unsigned int as an argument.
         */
        Message(Command c, unsigned int value) :
            command_(c), value_(value) {}
        /** 
         * Returns the value. Useful for the SELECT_CLIP command.
         */
        unsigned int get_value() { return value_; }
        /** 
         * Returns the Command id.
         */
        Command get_command() { return command_; }
    private:
        Command command_;
        // TODO:2010-10-03:aalex:Maybe sometimes the value will not be an unsigned int.
        unsigned int value_;
};

#endif // _MESSAGE_H_

