/*
 * Toonloop
 *
 * Copyright (c) 2010 Alexandre Quessy <alexandre@quessy.net>
 * Copyright (c) 2010 Tristan Matthews <le.businessman@gmail.com>
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
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <clutter-gst/clutter-gst.h>
#include <clutter-gtk/clutter-gtk.h>
#include <cstdlib> // for getenv
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h> // for snprintf
#include <string>
#include <tr1/memory>

#include "application.h"
#include "clip.h"
#include "config.h"
#include "configuration.h"
#include "controller.h"
#include "gui.h"
#include "midiinput.h"
#include "moviesaver.h"
#include "oscinterface.h"
#include "pipeline.h"
#include "statesaving.h"
#include "subprocess.h"
#include "unused.h"
#include "v4l2util.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
typedef std::tr1::unordered_map<int, std::tr1::shared_ptr<Clip> >::iterator ClipIterator;

namespace 
{
/**
 * Checks if a directory exists, create it and its parent directories if not.
 *
 * Returns whether the directory exists or not once done.
 * This is in an anonymous namespace to keep it visible to this file only and to ensure 
 * it doesn't conflict with a function we link in.
 */
bool make_sure_directory_exists(const std::string &directory)
{
    if (not fs::exists(directory))
    {
        try 
        {
            fs::create_directories(directory); // TODO: check if it returns true
        } 
        catch(const std::exception& e) 
        {
            // TODO: be more specific to fs::basic_filesystem_error<Path> 
            std::cerr << "An error occured while creating the directory: " << e.what() << std::endl;
            return false;
        }
    } 
    else 
    {
        if (not fs::is_directory(directory))
        {
            std::cout << "Error: " << directory << " is not a directory." << std::endl;
            return false;
        }
    }
    return true;
}

} // end of anonymous namespace

Application::Application() : 
        selected_clip_(0)
{
    // FIXME:2010-08-05:aalex:We should not create clips at startup like that.
    // They should be created on-demand
    using std::tr1::shared_ptr;
    for (unsigned int i = 0; i < MAX_CLIPS; i++)
        clips_[i] = shared_ptr<Clip>(new Clip(i));
}

// TODO:2010-12-18:aalex:Should we return shared_ptr<Clip>, not Clip*?
Clip* Application::get_current_clip()
{
    return clips_[selected_clip_].get();
}

Clip* Application::get_clip(unsigned int clip_number)
{
    if (clip_number > (clips_.size() - 1))
    {
        g_critical("Tried to get_clip %d", clip_number);
        return 0;
    }
    else 
    {
        return clips_[clip_number].get();
    }
}

unsigned int Application::get_current_clip_number()
{
    return selected_clip_;
}

/** Should be called only via calling the Controller::choose_clip method.
 */
void Application::set_current_clip_number(unsigned int clip_number)
{
    if (clip_number > (clips_.size() - 1))
        g_critical("Tried to set_current_clip_number to %d", clip_number);
    else 
    {
        selected_clip_ = clip_number;
        if (config_->get_verbose())
            std::cout << "current clip is " << selected_clip_ << std::endl;
    }
}

static void check_for_mencoder()
{
    std::string command("mencoder -list-options");
    bool ret = subprocess::run_command(command);
    if (! ret)
        g_critical("Could not find mencoder\n");
}

/**
 * Parses the command line and runs the application.
 */
void Application::run(int argc, char *argv[])
{
    std::string video_source = "/dev/video0";
    std::string project_home = DEFAULT_PROJECT_HOME;
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Show this help message and exit")
        ("project-home,H", po::value<std::string>()->default_value(project_home), "Path to the saved files")
        ("version", "Show program's version number and exit")
        ("verbose,v", po::bool_switch(), "Enables a verbose output")
        ("display,D", po::value<std::string>()->default_value(std::getenv("DISPLAY")), "Sets the X11 display name")
        ("playhead-fps", po::value<int>()->default_value(12), "Sets the initial playback rate of clips")
        ("fullscreen,f", po::bool_switch(), "Runs in fullscreen mode")
        ("video-source,d", po::value<std::string>()->default_value(video_source), "Sets the video source or device. Use \"test\" for color bars. Use \"x\" to capture the screen")
        ("midi-input,m", po::value<int>(), "Sets the input MIDI device number to open")
        ("list-midi-inputs,L", po::bool_switch(), "Lists MIDI inputs devices and exits")
        ("list-cameras,l", po::bool_switch(), "Lists connected cameras and exits")
        ("osc-receive-port,p", po::value<std::string>(), "Sets the listening OSC port")
        ("osc-send-port,P", po::value<std::string>(), "Sets the port to send OSC messages to")
        ("osc-send-addr,a", po::value<std::string>()->default_value("localhost"), "Sets the IP address to send OSC messages to")
        ("enable-mouse-controls,M", po::bool_switch(), "Enables simple controls with the mouse.")
        ("width", po::value<int>()->default_value(DEFAULT_CAPTURE_WIDTH), "Image capture width")
        ("height", po::value<int>()->default_value(DEFAULT_CAPTURE_HEIGHT), "Image capture height")
        ("max-images-per-clip", po::value<int>()->default_value(0), "If not zero, sets a maximum number of images per clip. The first image is then removed when one is added.")
        ("enable-intervalometer,i", po::bool_switch(), "Enables the intervalometer for the default clip at startup.")
        ("intervalometer-rate,I", po::value<float>()->default_value(10.0), "Sets the default intervalometer rate.")
        ("layout", po::value<unsigned int>()->default_value(0), "Sets the layout number.") // TODO:2010-10-05:aalex:Print the NUM_LAYOUTS
        ("remove-deleted-images", po::bool_switch(), "Enables the removal of useless image files.")
        ("enable-shaders,S", po::bool_switch(), "Enables GLSL shader effects.")
        ("enable-info-window,I", po::bool_switch(), "Enables a window for information text.")
        ("image-on-top", po::value<std::string>()->default_value(""), "Shows an unscaled image on top of all.")
        ("enable-preview-window", po::bool_switch(), "Enables a preview of the live camera feed.")
        ("print-properties", po::bool_switch(), "Prints a list of the Toonloop properties once running.")
        ("no-load-project", po::bool_switch(), "Disables project file loading.")
        ;
    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, desc), options);
    po::notify(options);
    
    // tmp: 
    bool verbose = options["verbose"].as<bool>();
    // Options that makes the program exit:
    if (options.count("help"))
    {
        // TODO: also print clutter-gst command line help
        std::cout << desc << std::endl;
        //std::cout << INTERACTIVE_HELP << std::endl;
        //std::cout << std::endl;
        return;
    }
    if (options.count("version"))
    {
        std::cout << PACKAGE << " " << PACKAGE_VERSION << std::endl;
        return; 
    }
    if (options["list-cameras"].as<bool>())
    {
        std::cout << "List of cameras:" << std::endl;
        v4l2util::listCameras();
        return; 
    }
    if (options["list-midi-inputs"].as<bool>())
    {
        MidiInput tmp_midi_input(this, verbose); 
        tmp_midi_input.enumerate_devices();
        return; 
    }
    // Options to use in the normal mode:
    if (options.count("video-source"))
    {
        video_source = options["video-source"].as<std::string>();
        if (video_source != "test" && video_source != "x")
        {
            if (! fs::exists(video_source))
            {
                std::cout << "Could not find device " << video_source << "." << std::endl;
                std::cout << "Using the test source." << std::endl;
                video_source = "test";
                // exit(1); // exit with error
            }
        }
        if (verbose)
            std::cout << "video-source is set to " << video_source << std::endl;
    }
    if (options.count("project-home")) // of course it will be there.
    {
        project_home = options["project-home"].as<std::string>();
        if (project_home == DEFAULT_PROJECT_HOME) // FIXME: replace ~ by $HOME instead of hard-coding this
            project_home = std::string(std::getenv("HOME")) + "/Documents/toonloop/default";
        if (verbose)
            std::cout << "project-home is set to " << project_home << std::endl;
        if (! setup_project_home(project_home))
            exit(1);
    }
    // FIXME: From there, the options are set in configuration.cpp
    // TODO: We should do this in only one place. 
#if 0
    if (options["intervalometer-on"].as<bool>())
        std::cout << "Intervalometer is on. (but not yet implemented)" << std::endl;
    if (options.count("intervalometer-interval"))
        std::cout << "The rate of the intervalometer is set to " << options["intervalometer-interval"].as<double>() << std::endl; 
#endif
    if (verbose)
        std::cout << "The initial frame rate for clip playhead is set to " << options["playhead-fps"].as<int>() << std::endl;
    if (options.count("playhead-fps"))
    { 
        for (ClipIterator iter = clips_.begin(); iter != clips_.end(); ++iter)
            iter->second.get()->set_playhead_fps(options["playhead-fps"].as<int>());
    }

    if (options["fullscreen"].as<bool>())
    {
        if (verbose)
            std::cout << "Fullscreen mode is on: " << options["fullscreen"].as<bool>() << std::endl;
    }
    
    // Stores the options in the Configuration class.
    //Configuration config(options);
    // A lot of options parsing is done in the constructor of Configuration:
    config_.reset(new Configuration(options));
    controller_.reset(new Controller(this));
    // It's very important to call set_project_home and set_video_source here:
    config_->set_project_home(project_home);
    update_project_home_for_each_clip();
    config_->set_video_source(video_source);
    
    // Start OSC
    if (config_->get_osc_recv_port() != OSC_PORT_NONE)
    {
        //TODO:2010-08-26:aalex:Replace atoi by something more sturdy
        int osc_recv_port_num = atoi(config_->get_osc_recv_port().c_str());
        if (osc_recv_port_num == 0)
        {
            std::cerr << "Invalid receiving port number: " << config_->get_osc_recv_port() << std::endl;
            exit(1);
        } else {
            if (osc_recv_port_num > 65535)
            {
                std::cerr << "Receiving port number " << config_->get_osc_recv_port() <<  "is over the maximum of 65535." <<  std::endl;
                exit(1);
            } else if (osc_recv_port_num < 1) {
                std::cerr << "Receiving port number " << config_->get_osc_recv_port() <<  "is under the minimum of 1." <<  std::endl;
                exit(1);
            } else if (osc_recv_port_num < 1024) {
                std::cerr << "Receiving port number " << config_->get_osc_recv_port() <<  "is under 1024. It will probably fail." <<  std::endl;
                // Don't exit
            }
        }
        if (verbose)
            std::cout << "Starting OSC receiver." << std::endl;
    } 
    else 
    {
        if (verbose)
            std::cout << "OSC receiver is disabled" << std::endl;
    }
    if (config_->get_osc_send_port() != OSC_PORT_NONE)
    {
        //TODO:2010-08-26:aalex:Replace atoi by something more sturdy
        int osc_send_port_num = atoi(config_->get_osc_send_port().c_str());
        std::cout << "OSC send port " << osc_send_port_num << std::endl; 
        if (osc_send_port_num == 0)
        {
            std::cerr << "Invalid sending port number: " << config_->get_osc_send_port() << std::endl;
            exit(1);
        }
        if (osc_send_port_num > 65535)
        {
            std::cerr << "Sending port number " << config_->get_osc_send_port() <<  "is over the maximum of 65535." <<  std::endl;
            exit(1);
        }
    } 
    else 
    {
        if (verbose)
            std::cout << "OSC sender is disabled" << std::endl;
    }
    osc_.reset(new OscInterface(this, config_->get_osc_recv_port(), config_->get_osc_send_port(), config_->get_osc_send_addr()));
    if (config_->get_osc_recv_port() != OSC_PORT_NONE || config_->get_osc_send_port() != OSC_PORT_NONE)
    {
        osc_->start();
    }

    movie_saver_.reset(new MovieSaver);
    // TODO: create a directory for clips and one for images.
    movie_saver_->set_result_directory(config_->get_project_home() + "/" + MOVIES_DIRECTORY);
    // Init GTK, Clutter and GST:
    GError *error;
    error = NULL;
    gtk_clutter_init(&argc, &argv);
    if (error)
        g_error("Unable to initialize Clutter: %s", error->message);
    clutter_gst_init(&argc, &argv);
    // start GUI
    if (verbose)
        std::cout << "Starting GUI." << std::endl;
    gui_.reset(new Gui(this));
    // start Pipeline
    if (verbose)
        std::cout << "Starting pipeline." << std::endl;
    pipeline_.reset(new Pipeline(this));
    // Start MIDI
    if (verbose)
        std::cout << "Starting MIDI input." << std::endl;
    midi_input_.reset(new MidiInput(this, verbose));
    if (verbose)
    {
        midi_input_->enumerate_devices();
        midi_input_->set_verbose(true);
    }
    if (config_->get_midi_input_number() != MIDI_INPUT_NONE)
    {
        bool midi_ok = midi_input_->open(config_->get_midi_input_number());
        if (midi_ok)
            std::cout << "MIDI: Opened port " << config_->get_midi_input_number() << std::endl;
        else
            std::cout << "MIDI: Failed to open port " << config_->get_midi_input_number() << std::endl;
    }
    // Sets the intervalometer stuff.
    if (verbose)
        std::cout << "Set the default intervalometer rate" << std::endl;
    for (ClipIterator iter = clips_.begin(); iter != clips_.end(); ++iter)
        iter->second.get()->set_intervalometer_rate(config_->get_default_intervalometer_rate());
    if (options["enable-intervalometer"].as<bool>())
    {
        if (verbose)
            std::cout << "starting the intervalometer" << std::endl;
        get_controller()->toggle_intervalometer();
    }

    // Sets the remove_deleted_images thing
    for (ClipIterator iter = clips_.begin(); iter != clips_.end(); ++iter)
        iter->second.get()->set_remove_deleted_images(config_->get_remove_deleted_images());

    // Clip size must be inherited...
    for (ClipIterator iter = clips_.begin(); iter != clips_.end(); ++iter)
    {
        iter->second.get()->set_width(config_->get_capture_width());
        iter->second.get()->set_height(config_->get_capture_height());
    }
    // Choose layout
    unsigned int layout = options["layout"].as<unsigned int>();
    if (layout < NUM_LAYOUTS)
        gui_->set_layout((Gui::layout_number) layout);
    else
        std::cout << "There is no layout number " << layout << ". Using 0." << std::endl;
    if (verbose)
        std::cout << "Check for mencoder" << std::endl;
    check_for_mencoder();
    // Print properties
    if (options["print-properties"].as<bool>())
        get_controller()->print_properties();

    if (! options["no-load-project"].as<bool>())
    {
        std::string project_file_name = config_->get_project_home() + "/" + statesaving::FILE_NAME;
        if (verbose)
            std::cout << "Checking for project file " << project_file_name << "..." << std::endl;
        if (fs::exists(project_file_name))
            load_project(project_file_name);
    }
    // Run the main loop
    // This call is blocking:
    // Starts it all:
    if (verbose)
        std::cout << "Running toonloop" << std::endl;
    gtk_main();
}

/**
 * Destructor of a toon looper.
 */
Application::~Application()
{
}

/**
 * Creates all the project directories.
 *
 * Returns whether the directories exist or not once done.
 */
bool Application::setup_project_home(const std::string &project_home)
{
    if (not make_sure_directory_exists(project_home))
        return false;
    if (not make_sure_directory_exists(project_home + "/" + MOVIES_DIRECTORY))
        return false;
    if (not make_sure_directory_exists(project_home + "/" + IMAGES_DIRECTORY))
        return false;
    return true;
}

void Application::update_project_home_for_each_clip()
{
    // TODO: be able to change the project_home on-the-fly
    for (ClipIterator iter = clips_.begin(); iter != clips_.end(); ++iter)
        iter->second.get()->set_directory_path(get_configuration()->get_project_home());
}

Pipeline* Application::get_pipeline() 
{
    return pipeline_.get();
}

Gui* Application::get_gui() 
{
    return gui_.get();
}

Controller* Application::get_controller() 
{
    return controller_.get();
}

Configuration* Application::get_configuration() 
{
    return config_.get();
}

MidiInput* Application::get_midi_input() 
{
    return midi_input_.get();
}

MovieSaver* Application::get_movie_saver() 
{
    return movie_saver_.get();
}

OscInterface* Application::get_osc_interface() 
{
    return osc_.get();
}

void Application::quit()
{
    if (config_->get_verbose())
        std::cout << "Quitting the application." << std::endl;
    pipeline_->stop();
    gtk_main_quit();
}

/**
 * Checks for asynchronous messages.
 * 
 * Useful for the MIDI and OSC controls.
 */
void Application::check_for_messages()
{
    // TODO: move message handling here.
    get_midi_input()->consume_commands();    
    get_osc_interface()->consume_commands();    
}

namespace
{
    bool node_name_is(xmlNode *node, const std::string &name)
    {
        return (node->type == XML_ELEMENT_NODE && node->name && (xmlStrcmp(node->name, XMLSTR name.c_str()) == 0));
    }

    /** Returns a pointer to the XML child with the given name
     * @return A pointer to the data, not a copy of it.
     */
    xmlNode *seek_child_named(xmlNode *parent, const std::string &name)
    {
        if (parent == NULL)
            return NULL;
        for (xmlNode *node = parent->children; node != NULL; node = node->next)
        {
            if (node_name_is(node, name))
            {
                return node;
            }
        }
        return NULL;
    }
}

bool Application::load_project(std::string &file_name)
{
    namespace ss = statesaving;
    // TODO: clear all the clips
    // parse the file and get the DOM tree
    xmlDoc *doc = xmlReadFile(file_name.c_str(), NULL, 0);
    if (doc == NULL)
    {
        printf("error: could not parse file %s\n", file_name.c_str());
        return false;
    }
    else
        std::cout << "Loading project file " << file_name << std::endl;
    xmlNode *root = xmlDocGetRootElement(doc);
    xmlNode *clips_node = seek_child_named(root, ss::CLIPS_NODE);
    if (clips_node != NULL)
    {
        //std::cout << "found clips node\n";
        for (xmlNode *clip_node = clips_node->children; clip_node; clip_node = clip_node->next)
        {
            // clip:
            if (node_name_is(clip_node, ss::CLIP_NODE))
            {
                //std::cout << "clip node: " << clip_node->name << std::endl;
                xmlChar *clip_id = xmlGetProp(clip_node, XMLSTR ss::CLIP_ID_PROPERTY);
                Clip *clip = 0;
                if (clip_id != NULL)
                {
                    try
                    {
                        unsigned int clip_number = boost::lexical_cast<unsigned int>(clip_id);
                        if (config_->get_verbose())
                            printf("Clip ID: %d \n", clip_number);
                        clip = get_clip(clip_number);
                    }
                    catch (boost::bad_lexical_cast &)
                    {
                        g_critical("Invalid int for %s in XML file: %s", clip_id, file_name.c_str());
                    }
                }
                xmlFree(clip_id); // free the property string
                if (clip)
                {
                    // images:
                    xmlNode *images_node = seek_child_named(clip_node, ss::IMAGES_NODE);
                    if (images_node != NULL)
                    {
                        for (xmlNode *image_node = images_node->children; image_node; image_node = image_node->next)
                        {
                            // image:
                            if (node_name_is(image_node, ss::IMAGE_NODE))
                            {
                                // image name
                                xmlChar *image_name = xmlGetProp(image_node, XMLSTR ss::IMAGE_NAME_ATTR);
                                if (image_name != NULL)
                                {
                                    if (config_->get_verbose())
                                        printf(" * Image name: %s\n", image_name);
                                    clip->add_image((char *) image_name);
                                }
                                xmlFree(image_name); // free the property string
                            }
                        } // end of image
                    } // end of images

                    // FPS:
                    xmlChar *fps_value = xmlGetProp(clip_node, XMLSTR ss::CLIP_FPS_PROPERTY);
                    if (fps_value != NULL)
                    {
                        try
                        {
                            unsigned int fps = boost::lexical_cast<unsigned int>(fps_value);
                            if (config_->get_verbose())
                                printf("Clip FPS: %d \n", fps);
                            clip->set_playhead_fps(fps);
                        }
                        catch (boost::bad_lexical_cast &)
                        {
                            g_critical("Invalid int for %s in XML file: %s", fps_value, file_name.c_str());
                        }
                    }
                    xmlFree(fps_value); // free the property string

                    // direction:
                    xmlChar *direction_value = xmlGetProp(clip_node, XMLSTR ss::CLIP_DIRECTION_PROPERTY);
                    if (direction_value != NULL)
                    {
                        if (config_->get_verbose())
                            printf("Clip direction: %s \n", (char *) direction_value);
                        bool ok = clip->set_direction((char *) direction_value);
                        if (! ok)
                            g_critical("Invalid direction name: %s", (char *) direction_value);
                    }
                    xmlFree(direction_value); // free the property string
                } // end of valid Clip*
            } // end of clip
        }
    } // end of clips
    // Free the document + global variables that may have been allocated by the parser.
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return true;
}

bool Application::save_project(std::string &file_name)
{
    namespace ss = statesaving;

    xmlDocPtr doc = xmlNewDoc(XMLSTR "1.0");
    // "project" node with its "name" attribute
    xmlNodePtr root_node = xmlNewNode(NULL, XMLSTR ss::ROOT_NODE);
    xmlDocSetRootElement(doc, root_node);
    xmlNewProp(root_node, XMLSTR ss::PROJECT_NAME_ATTR, XMLSTR ss::DEFAULT_PROJECT_NAME);
    xmlNewProp(root_node, XMLSTR ss::PROJECT_VERSION_ATTR, XMLSTR PACKAGE_VERSION);
    // "clips" node
    xmlNodePtr clips_node = xmlNewChild(root_node, NULL, XMLSTR ss::CLIPS_NODE, NULL); // No text contents

    char buff[256]; // buff for node names
    for (ClipIterator iter = clips_.begin(); iter != clips_.end(); ++iter)
    {
        Clip *clip = iter->second.get();
        if (clip->size() > 0) // do not save empty clips
        {
            xmlNodePtr clip_node = xmlNewChild(clips_node, NULL, XMLSTR ss::CLIP_NODE, NULL);
            // clip ID:
            sprintf(buff, "%d", clip->get_id());
            xmlNewProp(clip_node, XMLSTR ss::CLIP_ID_PROPERTY, XMLSTR buff);
            // clip FPS:
            sprintf(buff, "%d", clip->get_playhead_fps());
            xmlNewProp(clip_node, XMLSTR ss::CLIP_FPS_PROPERTY, XMLSTR buff);
            // clip direction:
            xmlNewProp(clip_node, XMLSTR ss::CLIP_DIRECTION_PROPERTY, XMLSTR clip->get_direction().c_str());
            // images:
            xmlNodePtr images_node = xmlNewChild(clip_node, NULL, XMLSTR ss::IMAGES_NODE, NULL);
            for (unsigned int image_num = 0; image_num < clip->size(); image_num++)
            {
                Image *image = clip->get_image(image_num);
                xmlNodePtr image_node = xmlNewChild(images_node, NULL, XMLSTR ss::IMAGE_NODE, NULL);
                xmlNewProp(image_node, XMLSTR ss::IMAGE_NAME_ATTR, XMLSTR image->get_name().c_str());
            }
        }
    }

    // Save document to file
    xmlSaveFormatFileEnc(file_name.c_str(), doc, "UTF-8", 1);
    if (config_->get_verbose())
        std::cout << "Saved the project to " << file_name << std::endl;
    // Free the document + global variables that may have been allocated by the parser.
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return true;
}

