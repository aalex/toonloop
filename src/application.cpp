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
#include "application.h"
#include "gui.h"
#include "pipeline.h"
#include "pipeline.h"
#include <iostream>
#include <string>
#include <gtk/gtk.h>
#include <boost/program_options.hpp>
#include <cstdlib> // for getenv
#include "./config.h"
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gtk/gtkgl.h>

namespace po = boost::program_options;

Application* Application::instance_ = 0;

Application::Application() 
{}

void Application::run(int argc, char *argv[])
{
    //std::cout << "Running toonloop" << std::endl;
    // FIXME: parse_options(argc, argv);
    // Init GTK:
    gtk_init(&argc, &argv);
    // Init GTK GL:
    gtk_gl_init(&argc, &argv);
    // Init GST:
    gst_init(&argc, &argv);
    start_gui();
    start_pipeline();
    get_pipeline().set_drawing_area(get_gui().get_drawing_area());
    gtk_main();
}

// FIXME: still causing errors, so we're not using it.
void Application::parse_options(int argc, char *argv[])
{
    std::cout << "Parsing options." << std::endl;
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
    po::store(po::parse_command_line(argc, argv, desc), options_);
    po::notify(options_);
    if (options_.count("help"))
    {
        std::cout << desc << std::endl;
        return;
    }
    if (options_.count("version"))
    {
        std::cout << PACKAGE << " " << PACKAGE_VERSION << std::endl;
        return; 
    }
    if (options_.count("video-device"))
        std::cout << "video-device is set to " << options_["video-device"].as<std::string>() << std::endl;
    if (options_["intervalometer-on"].as<bool>())
        std::cout << "Intervalometer is on" << std::endl;
    if (options_.count("intervalometer-interval"))
        std::cout << "The rate of the intervalometer is set to " << options_["intervalometer-rate"].as<std::string>() << std::endl;
    if (options_.count("project-name"))
        std::cout << "The project name is set to " << options_["project-name"].as<std::string>() << std::endl;
    if (options_.count("fps"))
        std::cout << "The frame rate is set to " << options_["fps"].as<std::string>() << std::endl;
    if (options_["fullscreen"].as<bool>())
        std::cout << "Fullscreen mode is on" << std::endl;
}

Pipeline& Application::get_pipeline() 
{
    return *pipeline_;
}
Gui& Application::get_gui() 
{
    return *gui_;
}

Application& Application::get_instance()
{
    if (!instance_)
        instance_ = new Application();
    return *instance_;
}

void Application::start_gui()
{
    std::cout << "Starting GUI." << std::endl;
    gui_ = std::tr1::shared_ptr<Gui>(new Gui());
}

void Application::start_pipeline()
{
    std::cout << "Starting pipeline." << std::endl;
    pipeline_ = std::tr1::shared_ptr<Pipeline>(new Pipeline());
}

// delete the instance, not sure how safe this is
void Application::reset()
{
    std::cout << "Resetting the application" << std::endl;
    delete instance_;
    instance_ = 0;
}

void Application::quit()
{
    std::cout << "Quitting the application." << std::endl;
    get_pipeline().stop();
    gtk_main_quit();
}
