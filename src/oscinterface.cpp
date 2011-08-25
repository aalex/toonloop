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

#include <boost/bind.hpp>
#include <cstdio>
#include <iostream>
#include <lo/lo.h>
#include <tr1/memory>

#include "application.h"
#include "command.h"
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
    owner_(owner),
    messaging_queue_()
{
    if (listen_port != OSC_PORT_NONE)
        receiving_enabled_ = true;
    if (send_port != OSC_PORT_NONE)
        sending_enabled_ = true;
    if (receiving_enabled_)
    {
        std::cout << "Listening osc_udp://localhost:" << listen_port << std::endl;
        receiver_.addHandler("/ping", "", ping_cb, this);
        receiver_.addHandler("/pong", "", pong_cb, this);
        receiver_.addHandler("/toon/quit", "", quit_cb, this);
        receiver_.addHandler("/toon/frame/add", "", add_image_cb, this);
        receiver_.addHandler("/toon/frame/remove", "", remove_image_cb, this);
        receiver_.addHandler("/toon/clip/select", "i", select_clip_cb, this);
        receiver_.addHandler("/toon/clip/save_current", "", clip_save_current_cb, this);
        receiver_.addHandler("/toon/clip/import_image", "s", import_image_cb, this);
        receiver_.addHandler("/toon/set_int", "si", set_int_cb, this);
        receiver_.addHandler("/toon/set_float", "sf", set_float_cb, this);
        std::cout << "OSC message handlers:" << std::endl;
        std::cout << " * /ping : Answers with /pong" << std::endl;
        std::cout << " * /pong" << std::endl;
        std::cout << " * /toon/quit : Quits" << std::endl;
        std::cout << " * /toon/frame/add : Grabs a frame" << std::endl;
        std::cout << " * /toon/frame/remove : Removes a frame" << std::endl;
        std::cout << " * /toon/clip/select i:clip_number : Selects a clip" << std::endl;
        std::cout << " * /toon/clip/save_current : Saves the currently selected clip" << std::endl;
        std::cout << " * /toon/clip/import_image s:file_name: Imports an image" << std::endl;
        std::cout << " * /toon/set_int s:property_name i:value: Sets an int property value" << std::endl;
        std::cout << " * /toon/set_float s:property_name f:value: Sets a float property value" << std::endl;
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
    // new ones:
    controller->choose_clip_signal_.connect(boost::bind(
        &OscInterface::on_choose_clip, this, _1));
    controller->clip_fps_changed_signal_.connect(boost::bind(
        &OscInterface::on_clip_fps_changed, this, _1, _2));
    controller->save_clip_signal_.connect(boost::bind(
        &OscInterface::on_clip_saved, this, _1, _2));
    controller->no_image_to_play_signal_.connect(boost::bind(
        &OscInterface::on_no_image_to_play, this));
    controller->clip_direction_changed_signal_.connect(boost::bind(
        &OscInterface::on_clip_direction_changed, this, _1, _2));
    controller->clip_cleared_signal_.connect(boost::bind(
        &OscInterface::on_clip_cleared, this, _1));
}

/** Slot for Controller::choose_clip_signal_
 *
 * Send /toon/clip/select i:clip_number
 * */
void OscInterface::on_choose_clip(unsigned int clip_number)
{
    sender_.sendMessage("/toon/clip/select", "i", (int) clip_number, LO_ARGS_END);
}

/** Slot for Controller::clip_fps_changed_signal_
 *
 * Send /toon/clip/fps i:clip_number i:fps
 * */
void OscInterface::on_clip_fps_changed(unsigned int clip_number, unsigned int fps)
{
    sender_.sendMessage("/toon/clip/fps", "ii", (int) clip_number, fps, LO_ARGS_END);
}

/** Slot for Controller::save_clip_signal_
 *
 * Send /toon/clip/saved i:clip_number s:file_name
 * */
void OscInterface::on_clip_saved(unsigned int clip_number, std::string file_name)
{
    sender_.sendMessage("/toon/clip/saved", "is", (int) clip_number, file_name.c_str(), LO_ARGS_END);
}

/** Slot for Controller::no_image_to_play_signal_
 *
 * Send /toon/playhead/none
 * */
void OscInterface::on_no_image_to_play()
{
    sender_.sendMessage("/toon/playhead/none", "", LO_ARGS_END);
}

/** Slot for Controller::clip_direction_changed_signal_
 *
 * Send /toon/clip/direction i:clip_number s:direction
 * */
void OscInterface::on_clip_direction_changed(unsigned clip_number, std::string direction)
{
    sender_.sendMessage("/toon/clip/direction", "ii", (int) clip_number, direction.c_str(), LO_ARGS_END);
}

/** Slot for Controller::clip_cleared_signal_
 *
 * Send /toon/clip/cleared i:clip_number
 * */
void OscInterface::on_clip_cleared(unsigned int clip_number)
{
    sender_.sendMessage("/toon/clip/cleared", "i", (int) clip_number, LO_ARGS_END);
}

/**
 * Handles /ping. Answers with /pong
 *
 * \msc
 * Client,Toonloop;
 * Client->Toonloop [ label = "/ping" ];
 * Toonloop->Client [ label = "/pong" ];
 * \endmsc
 */
int OscInterface::ping_cb(
        const char *path, 
        const char * /*types*/, lo_arg ** /*argv*/,
        int /*argc*/, void * /*data*/, void *user_data)
{ 
    UNUSED(path);
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path << std::endl;
    if (context->sending_enabled_)
        context->sender_.sendMessage("/pong", "", LO_ARGS_END);
    return 0;
} 

/**
 * Handles /pong. Does nothing.
 */
int OscInterface::pong_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path << std::endl;
    return 0;
}
int OscInterface::add_image_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path << std::endl;
    context->push_command(std::tr1::shared_ptr<Command>(new AddImageCommand));
    return 0;
}

int OscInterface::import_image_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg **argv,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    std::string file_name(static_cast<const char*>(&argv[0]->s));
    if (context->is_verbose())
        std::cout << "Got " << path << " " << file_name << std::endl;
    context->push_command(std::tr1::shared_ptr<Command>(new ImportImageCommand(file_name)));
    return 0;
}

/**
 * Handles /toon/frame/remove ,ii
 */
int OscInterface::remove_image_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path <<  std::endl;
    context->push_command(std::tr1::shared_ptr<Command>(new RemoveImageCommand));
    return 0;
}

bool OscInterface::is_verbose()
{
    return owner_->get_configuration()->get_verbose();
}

// TODO:2010-08-15:aalex:Should be able to disable this handler
/** Handles /toon/quit
 *
 * Quits the application.
 */
int OscInterface::quit_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path << std::endl;
    context->push_command(std::tr1::shared_ptr<Command>(new QuitCommand));
    return 0;
}

/** Handles /toon/clip/select i:clip_number
 *
 * Selects a clip
 */
int OscInterface::select_clip_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg **argv,
        int /* argc */, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    unsigned int clip_number = (unsigned int) argv[0]->i;
    if (context->is_verbose())
        std::cout << "Got " << path << " i:" << clip_number << std::endl;
    context->push_command(std::tr1::shared_ptr<Command>(new SelectClipCommand(clip_number)));
    return 0;
}

/** Handles /toon/clip/save_current i:clip_number
 *
 * Saves the currently selected clip.
 */
int OscInterface::clip_save_current_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path << std::endl;
    context->push_command(std::tr1::shared_ptr<Command>(new SaveCurrentClipCommand));
    return 0;
}

/** Destructor 
 */
OscInterface::~OscInterface()
{
}

/** Starts listening if enabled
 */
void OscInterface::start()
{
    if (receiving_enabled_)
    {
        // start a thread to try and subscribe us
        receiver_.listen(); // start listening in separate thread
    }
}

/** Slot for Controller::add_frame_signal_ 
 *
 * Sends /toon/frame/add i:clip_number i:frame_number
 */
void OscInterface::on_add_frame(unsigned int clip_number, unsigned int frame_number)
{
    sender_.sendMessage("/toon/frame/add", "ii", clip_number, frame_number, LO_ARGS_END);
}

/** Slot for Controller::remove_frame_signal_ 
 *
 * Sends /toon/frame/remove i:clip_number i:frame_number
 */
void OscInterface::on_remove_frame(unsigned int clip_number, unsigned int frame_number)
{
    sender_.sendMessage("/toon/frame/remove", "ii", clip_number, frame_number, LO_ARGS_END);
}

/** Slot for Controller::next_image_to_play_signal_
 *
 * Sends /toon/clip/playhead i:clip_number i:image_number s:file_name
 */
void OscInterface::on_next_image_to_play(unsigned int clip_number, unsigned int image_number, std::string file_name)
{
    sender_.sendMessage("/toon/clip/playhead", "iis", clip_number, image_number, file_name.c_str(), LO_ARGS_END);
}

/**
 * Takes action!
 *
 * Should be called when it's time to take action, before rendering a frame, for example.
 */
void OscInterface::consume_commands()
{
    // TODO:2010-10-03:aalex:Move this handling to Application.
    bool success = true;
    while (success)
    {
        std::tr1::shared_ptr<Command> command;
        success = messaging_queue_.try_pop(command);
        if (success)
        {
            if (is_verbose())
                std::cout << __FUNCTION__ << ": apply " << command->get_name() << std::endl;
            command->apply(*(owner_->get_controller()));
        }
    }
}

void OscInterface::push_command(std::tr1::shared_ptr<Command> command)
{
    // TODO: pass this message argument by reference?
    messaging_queue_.push(command);
}

/**
 * Handles /toon/set_int ,si
 */
int OscInterface::set_int_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg **argv,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path <<  std::endl;
    std::string name(static_cast<const char*>(&argv[0]->s));
    int value = (unsigned int) argv[1]->i;
    context->push_command(std::tr1::shared_ptr<Command>(new SetIntCommand(name, value)));
    return 0;
}

/**
 * Handles /toon/set_float ,sf
 */
int OscInterface::set_float_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg **argv,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path <<  std::endl;
    std::string name(static_cast<const char*>(&argv[0]->s));
    float value = (float) argv[1]->f;
    context->push_command(std::tr1::shared_ptr<Command>(new SetFloatCommand(name, value)));
    return 0;
}

