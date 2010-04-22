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
// prototype for boost program options.
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include "./config.h"

namespace po = boost::program_options;

int run(int argc, char* argv[])
{
    po::options_description desc("Toonloop options");
    desc.add_options()
        ("help,h", "Show this help message and exit")
        ("version", "Show program's version number and exit")
        ("video-device,d", po::value<std::string>(), "Sets the video device name or number");
        //("display", po::value<string>(std::getenv("DISPLAY")), "Set the X11 DISPLAY variable.")
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0; // success
    }
    if (vm.count("version"))
    {
        std::cout << PACKAGE << " " << PACKAGE_VERSION << std::endl;
        return 0; // success
    }
    if (vm.count("video-device"))
    {
        std::cout << "video-device is set to " << vm["video-device"].as<std::string>() << std::endl;
    }
    return 0;
}


int main(int argc, char* argv[])
{
    return run(argc, argv);
}
