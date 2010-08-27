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

#ifndef __OSC_INTERFACE_H__
#define __OSC_INTERFACE_H__

#include <string>
#include "./oscreceiver.h"
#include "./oscsender.h"

class Application;
/** Open Sound Control sending and receiving for Toonloop.
 */
class OscInterface 
{
    public:
            OscInterface(
                    Application* owner, 
                    const std::string &listen_port,
                    const std::string &send_port,
                    const std::string &send_addr); 
            ~OscInterface();
            void start();
            /**
             * Slot for the Controller's add_frame_signal_ signal.
             */
            void on_add_frame(unsigned int clip_number, unsigned int frame_number);
            /**
             * Slot for the Controller's remove_frame_signal_ signal.
             */
            void on_remove_frame(unsigned int clip_number, unsigned int frame_number);
            /**
             * Slot for the Controller's next_image_to_play_signal_ signal.
             */
            void on_next_image_to_play(unsigned int clip_number, unsigned int image_number, std::string file_name);
    private:
        OscReceiver receiver_;
        OscSender sender_;
        bool sending_enabled_;
        bool receiving_enabled_;
        Application* owner_;
        //boost::mutex tryToSubscribeMutex_;
        //bool tryToSubscribe_;   // FIXME: should this be protected by a mutex?
        //void subscribe();
        static int pingCb(const char *path, 
                const char *types, lo_arg **argv, 
                int argc, void *data, void *user_data);
        static int pongCb(const char *path, 
                const char *types, lo_arg **argv, 
                int argc, void *data, void *user_data);
        static int quitCb(const char *path, 
                const char *types, lo_arg **argv, 
                int argc, void *data, void *user_data);
        static int addFrameCb(const char *path, 
                const char *types, lo_arg **argv, 
                int argc, void *data, void *user_data);
        static int removeFrameCb(const char *path, 
                const char *types, lo_arg **argv, 
                int argc, void *data, void *user_data);
        void connect_signals_to_sending_slots();
};

#endif // __OSC_INTERFACE_H__

