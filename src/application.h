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
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "gui.h"
#include "pipeline.h"
#include "clip.h"
#include <tr1/memory>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class Application 
{
    public:
        void run(int argc, char *argv[]);
        void quit();
        static void reset();
        Gui &get_gui();
        Pipeline &get_pipeline();
        static Application& get_instance();
        Clip* get_current_clip();
        double get_cfps();

    private:
        Application();
        static Application* instance_; // singleton
        std::tr1::shared_ptr<Gui> gui_;
        std::tr1::shared_ptr<Pipeline> pipeline_;
        int selected_clip_;
        double cfps_;
        std::tr1::unordered_map<int, Clip*> clips_;
};

#endif // __APPLICATION_H__
