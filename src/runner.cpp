// prototype for boost program options.
#include <iostream>
#include <string>
#include <cstdlib> // for getenv
#include <boost/program_options.hpp>
#include "./config.h"

namespace po = boost::program_options;

int run(int argc, char* argv[])
{
    po::options_description desc("Toonloop options");
    desc.add_options()
        ("help,h", "Show this help message and exit")
        ("toonloop-home,H", po::value<std::string>()->default_value(std::getenv("HOME")), "Path to the saved files")
        ("version", "Show program's version number and exit")
        ("verbose,v", po::bool_switch(), "Enables a verbose output")
        ("intervalometer-on,i", po::bool_switch(), "Enables the intervalometer to create time lapse animations")
        ("intervalometer-interval,I", po::value<float>()->default_value(5.0), "Sets the intervalometer rate in seconds")
        ("project-name,p", po::value<std::string>()->default_value("default"), "Sets the name of the project for image saving")
        ("display,D", po::value<std::string>()->default_value(std::getenv("DISPLAY")), "Sets the X11 display name")
        ("fps,f", po::value<int>()->default_value(30), "Rendering frame rate")
        ("image-size,s", po::value<std::string>()->default_value("640x480"), "Size of the images grabbed from the camera. Default is 640x480")
        ("fullscreen,F", po::bool_switch(), "Runs in fullscreen mode.")
        ("video-device,d", po::value<std::string>(), "Sets the video device name or number");
        //("display", po::value<string>(std::getenv("DISPLAY")), "Set the X11 DISPLAY variable.")
    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, desc), options);
    po::notify(options);
    if (options.count("help"))
    {
        std::cout << desc << std::endl;
        return 0; // success
    }
    if (options.count("version"))
    {
        std::cout << PACKAGE << " " << PACKAGE_VERSION << std::endl;
        return 0; // success
    }
    if (options.count("video-device"))
        std::cout << "video-device is set to " << options["video-device"].as<std::string>() << std::endl;
    if (options["intervalometer-on"].as<bool>())
        std::cout << "Intervalometer is on" << std::endl;
    if (options.count("intervalometer-interval"))
        std::cout << "The rate of the intervalometer is set to " << options["intervalometer-rate"].as<std::string>() << std::endl;
    if (options.count("project-name"))
        std::cout << "The project name is set to " << options["project-name"].as<std::string>() << std::endl;
    if (options.count("fps"))
        std::cout << "The frame rate is set to " << options["fps"].as<std::string>() << std::endl;
    if (options["fullscreen"].as<bool>())
        std::cout << "Fullscreen mode is on" << std::endl;
    return 0;
}


int main(int argc, char* argv[])
{
    return run(argc, argv);
}
