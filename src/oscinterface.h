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
    public:
            OscInterface(const std::string &listen_port); //,
                    //const std::string &send_host, 
                    //const std::string &send_port);
            ~OscInterface();
            void start();
            void publish_added_frame() const;
};

#endif // __OSC_INTERFACE_H__

