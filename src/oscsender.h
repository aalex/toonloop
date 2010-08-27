/** General-purpose wrapper around liblo to send OSC messages.
 */
#ifndef _OSC_SENDER_H_
#define _OSC_SENDER_H_

#include <lo/lo.h>
#include <string>

class OscSender {
    public:
        OscSender();
        OscSender(const std::string &host, const std::string &port);
        std::string toString() const;
        const char * host() { return host_.c_str(); }
        const char * port() { return port_.c_str(); }
        void sendMessage(const std::string &OSCpath, const char *types, ...) const;
    private:
        void sendMessage(const std::string &OSCpath, const char *types, va_list ap) const;
        void sendMessage(const std::string &OSCpath, lo_message msg) const;
        std::string host_;
        std::string port_;
        lo_address address_;
};

#endif // _OSC_SENDER_H_

