#include <iostream>
#include "./oscsender.h"

OscSender::OscSender() :
    host_(""), port_(""), 
    address_(0)
{}

OscSender::OscSender(const std::string &host, const std::string &port) :
    host_(host), port_(port), 
    address_(lo_address_new(host.c_str(), port_.c_str()))
{
}

std::string OscSender::toString() const
{
    return "host:" + host_ + ", port:" + port_;
}

void OscSender::sendMessage(const std::string &OSCpath, const char *types, ...) const
{
    va_list ap;
    va_start(ap, types);
    sendMessage(OSCpath, types, ap);
}

void OscSender::sendMessage(const std::string &OSCpath, const char *types, va_list ap) const
{
    lo_message msg = lo_message_new();
    int err = lo_message_add_varargs(msg, types, ap);

    if (!err)
        sendMessage(OSCpath, msg);
    else 
        std::cout << "ERROR (OscSender::sendMessage): " << err << std::endl;
}

void OscSender::sendMessage(const std::string &OSCpath, lo_message msg) const
{
    lo_send_message(address_, OSCpath.c_str(), msg);

    // Let's free the message after (not sure if this is necessary):
    lo_message_free(msg);
}

