#include "application.h"
#include "gui.h"
#include "pipeline.h"
#include "pipeline.h"
#include <iostream>
#include <gtk/gtk.h>

Application* Application::instance_ = 0;

Application::Application() 
{}

void Application::run()
{
    std::cout << "Running toonloop\n";
    start_gui();
    start_pipeline();
    get_pipeline().set_drawing_area(get_gui().get_drawing_area());
    gtk_main();
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
