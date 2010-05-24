/*
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  $Id$
 */

#include "oscinterface.h"
#include <cstdio>
#include <tr1/memory>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "lo/lo.h"

OscInterface::OscInterface(
        const std::string &listen_port)//,
        //const std::string &send_host,
        //const std::string &send_port) 
    :
    receiver_(listen_port) //,
    //sender_(serverHost, serverListenPort),
    //tryToSubscribe_(true)
{
    std::cout << "Listening osc_udp://localhost:" << listen_port << std::endl;
    receiver_.addHandler("/ping", "", pingCb, this);
}

int OscInterface::pingCb(const char *path, 
        const char *types, lo_arg **argv, 
        int argc, void *data, void *user_data) 
{ 
    OscInterface *context = static_cast<OscInterface*>(user_data);
#ifdef CONFIG_DEBUG
    std::cout << "Got " << path << std::endl << std::endl;
#endif
    std::cout << "Got ping" << std::endl;
    return 0;
} 

OscInterface::~OscInterface()
{
}

void OscInterface::start()
{
    // start a thread to try and subscribe us
    //boost::thread trySubscribe(boost::bind<void>(&StateClient::subscribe, this));
    receiver_.listen(); // start listening in separate thread
}

