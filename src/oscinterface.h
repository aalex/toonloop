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
#include <boost/thread.hpp>

//#include "./oscSender.h"
#include "./oscreceiver.h"

class OscInterface {
    private:
        OscReceiver receiver_;
        //OscSender sender_;
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
    public:
            OscInterface(const std::string &listen_port); //,
                    //const std::string &send_host, 
                    //const std::string &send_port);
            ~OscInterface();
            void start();
            void publish_added_frame() const;
};

#endif // __OSC_INTERFACE_H__

