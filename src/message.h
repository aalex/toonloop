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
 *
 * There are many constructor for this object, because it might have 
 * int float and string values associated with the message enum.
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
        };
        // Constructor with no arg
        Message() : 
            command_(), 
            value_(0), 
            float_value_(0.0),
            string_value_("")
        {}
        /**
         * Constructor. One must provide at least a command id.
         */
        Message(Command c) : 
            command_(c), 
            value_(0), 
            float_value_(0.0),
            string_value_("")
        {}
        /**
         * Constructor for commands which accept a unsigned int as an argument.
         */
        Message(Command c, unsigned int value) :
            command_(c), 
            value_(value), 
            float_value_(0.0),
            string_value_("")
        {}
        /**
         * Constructor for commands which accept a string and a float as arguments.
         */
        Message(Command c, std::string string_arg, float float_value) :
            command_(c),
            value_(0),
            float_value_(float_value), 
            string_value_(string_arg)
        {}
        /** 
         * Returns the value. Useful for the SELECT_CLIP command.
         */
        // TODO:2010-11-13:aalex:Rename to get_int_value
        unsigned int get_value() { return value_; }
        unsigned int get_int_value() { return value_; }
        float get_float_value() { return float_value_; }
        std::string &get_string_value() { return string_value_; }
        /** 
         * Returns the Command id.
         */
        Command get_command() { return command_; }
    private:
        Command command_;
        // TODO:2010-10-03:aalex:Maybe sometimes the value will not be an unsigned int.
        unsigned int value_;
        float float_value_;
        std::string string_value_;
};

#endif // _MESSAGE_H_

