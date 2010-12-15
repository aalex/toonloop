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

// TODO:2010-12-14:aalex: replace by a hierarchy of classes
// With an apply(Controller &controller) method
// and a different constructor for each.
// The MidiRule class could contain an instance of Message, and duplicate it on demand.
/*
class BaseMessage
{
    Message() {}
    virtual void apply(Application *application);
}
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
        };
        /**
         * Constructor. 
         */
        Message(Command c) : 
            command_(c), 
            int_value_(0), 
            float_value_(0.0),
            string_value_("")
        {}
        /** 
         * Returns the int value.
         */
        int get_int() { return int_value_; }
        /** 
         * Returns the float value.
         */
        float get_float() { return float_value_; }
        /** 
         * Returns the string value.
         */
        std::string &get_string() { return string_value_; }

        Message &set_int(int value) 
        {
            int_value_ = value; 
            return *this; 
        }
        Message &set_float(float value) 
        {
            float_value_ = value; 
            return *this; 
        }
        Message &set_string(const std::string &value) 
        { 
            string_value_ = value; 
            return *this; 
        }
        /** 
         * Returns the Command id.
         */
        Command get_command() { return command_; }
    private:
        Command command_;
        int int_value_;
        float float_value_;
        std::string string_value_;
};

#endif // _MESSAGE_H_

