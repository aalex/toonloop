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
#include <iostream>
#include <cstdlib>
#include "midi.h"
#include <unistd.h> // sleep

void usage() 
{
    // Error function in case of incorrect command-line
    // argument specifications.
    std::cout << "\nuseage: check_midi <port> <duration>\n";
    std::cout << "    port = the device to use (default = 0).\n\n";
    std::cout << "    duration = how long to sleep (default = 10) Use 0 to sleep forever.\n\n";
    exit(0);
}

void on_pedal_down()
{
    std::cout << "Pedal is down! (slot)" << std::endl;
}

int main( int argc, char *argv[] )
{
    unsigned int port = 0;
    unsigned int duration = 1; // Should it be more?
    MidiInput in; // 
    //in(); // = MidiInput(); // FIXME
    in.pedal_down_signal_.connect(&on_pedal_down);
    in.set_verbose(true);
    // Minimal command-line check.
    if (argc > 3) 
        usage();
    if (argc >= 2) 
        port = (unsigned int) atoi(argv[1]);
    if (argc == 3) 
        duration = (unsigned int) atoi(argv[2]);

    in.enumerate_devices();
    std::cout << "Opening " << port << std::endl;
    bool success = in.open(port);
    if (success) 
    {
        std::cout << "\nReading MIDI input from port " << port << "..." << std::endl;
        //char input;
        //std::cin.get(input);
        if (duration == 0)
        {
            std::cout << "Sleeping forever. Ctrl-C to quit..." << std::endl;
            while (true)
                sleep(1);
        } else {
            std::cout << "Sleeping for " << duration << " seconds..." << std::endl;
            while (duration > 0) 
            {
                std::cout << duration << " seconds left..." << std::endl;
                sleep(1);
                -- duration;
            }
            std::cout << "Done." << std::endl;
        }
    } else {
        std::cout << "Failed to open port " << port << std::endl;
    }
    return 0;
}

