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

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <cstdio>
#include <iostream>
#include <lo/lo.h>
#include <tr1/memory>

#include "application.h"
#include "controller.h"
#include "log.h"
#include "oscinterface.h"
#include "pipeline.h"
#include "unused.h"

OscInterface::OscInterface(
        Application* owner,
        const std::string &listen_port,
        const std::string &send_port, 
        const std::string &send_addr)
    :
    receiver_(listen_port),
    sender_(send_addr, send_port),
    sending_enabled_(false),
    receiving_enabled_(false),
    owner_(owner)
{
    if (listen_port != OSC_PORT_NONE)
        receiving_enabled_ = true;
    if (send_port != OSC_PORT_NONE)
        sending_enabled_ = true;
    if (receiving_enabled_)
    {
        std::cout << "Listening osc_udp://localhost:" << listen_port << std::endl;
        receiver_.addHandler("/ping", "", pingCb, this);
        receiver_.addHandler("/pong", "", pongCb, this);
        receiver_.addHandler("/toon/quit", "", quitCb, this);
        receiver_.addHandler("/toon/frame/add", "", addFrameCb, this);
        receiver_.addHandler("/toon/frame/remove", "", removeFrameCb, this);
        std::cout << "OSC message handlers:" << std::endl;
        std::cout << " * /ping : Answers with /pong" << std::endl;
        std::cout << " * /pong" << std::endl;
        std::cout << " * /toon/quit : Quits" << std::endl;
        std::cout << " * /toon/frame/add : Grabs a frame" << std::endl;
        std::cout << " * /toon/frame/remove : Removes a frame" << std::endl;
    }
    if (sending_enabled_)
    {
        std::cout << "Sending to osc_udp://" << send_addr << ":" << send_port << std::endl;
        connect_signals_to_sending_slots();
    }
}
/**
 * Connects the Controller's signals to this class' slots.
 */
void OscInterface::connect_signals_to_sending_slots()
{
    Controller* controller = owner_->get_controller();
    controller->add_frame_signal_.connect(boost::bind(
        &OscInterface::on_add_frame, this, _1, _2));
    controller->remove_frame_signal_.connect(boost::bind(
        &OscInterface::on_remove_frame, this, _1, _2));
    controller->next_image_to_play_signal_.connect(boost::bind(
        &OscInterface::on_next_image_to_play, this, _1, _2, _3));
}

int OscInterface::pingCb(
        const char *path, 
        const char * /*types*/, lo_arg ** /*argv*/,
        int /*argc*/, void * /*data*/, void * /*user_data*/)
{ 
#ifdef CONFIG_DEBUG
    std::cout << "Got " << path << std::endl << std::endl;
#endif
    UNUSED(path);
    std::cout << "Got ping" << std::endl;
    return 0;
} 

int OscInterface::pongCb(
        const char * /*path*/,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void * /*user_data*/)
{
    std::cout << "Got /pong" << std::endl;
    return 0;
}
int OscInterface::addFrameCb(
        const char * /*path*/,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    std::cout << "Got /toon/frame/add" << std::endl;
    OscInterface* context = static_cast<OscInterface*>(user_data);
    context->owner_->get_pipeline()->grab_frame();
    return 0;
}

int OscInterface::removeFrameCb(
        const char * /*path*/,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    std::cout << "Got /toon/frame/remove" << std::endl;
    OscInterface* context = static_cast<OscInterface*>(user_data);
    context->owner_->get_pipeline()->remove_frame();
    return 0;
}
/**
 * Handles /toon/quit
 *
 * Quits the application.
 */
// TODO:2010-08-15:aalex:Should be able to disable this handler
int OscInterface::quitCb(
        const char * /*path*/,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    std::cout << "Got /toon/quit" << std::endl;
    OscInterface* context = static_cast<OscInterface*>(user_data);
    context->owner_->quit();
    return 0;
}

OscInterface::~OscInterface()
{
}

void OscInterface::start()
{
    if (receiving_enabled_)
    {
        // start a thread to try and subscribe us
        //boost::thread trySubscribe(boost::bind<void>(&StateClient::subscribe, this));
        receiver_.listen(); // start listening in separate thread
    }
}

void OscInterface::on_add_frame(unsigned int clip_number, unsigned int frame_number)
{
    UNUSED(clip_number);
    UNUSED(frame_number);
    LOG_DEBUG("OSC: on_add_frame");
}
void OscInterface::on_remove_frame(unsigned int clip_number, unsigned int frame_number)
{
    UNUSED(clip_number);
    UNUSED(frame_number);
    LOG_DEBUG("OSC: on_remove_frame");
}
void OscInterface::on_next_image_to_play(unsigned int clip_number, unsigned int image_number, std::string file_name)
{
    UNUSED(clip_number);
    UNUSED(image_number);
    UNUSED(file_name);
    // TODO: remove this OSC logging
    LOG_DEBUG("OSC: on_next_image_to_play");
}

