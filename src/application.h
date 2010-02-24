#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "gui.h"
#include "pipeline.h"
#include <tr1/memory>

class Application 
{
    public:
        void run();
        void start_gui();
        void start_pipeline();
        void quit();
        static void reset();
        Gui &get_gui();
        Pipeline &get_pipeline();
        static Application& get_instance();
    private:
        Application();
        static Application* instance_;
        std::tr1::shared_ptr<Gui> gui_;
        std::tr1::shared_ptr<Pipeline> pipeline_;
};

#endif // __APPLICATION_H__
