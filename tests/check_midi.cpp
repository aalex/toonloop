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
    std::cout << "\nuseage: cmidiin <port>\n";
    std::cout << "    where port = the device to use (default = 0).\n\n";
    exit(0);
}

int main( int argc, char *argv[] )
{
    unsigned int port = 0;
    MidiInput in = MidiInput(); // FIXME
    // Minimal command-line check.
    if (argc > 2) 
        usage();
    if (argc == 2) 
        port = (unsigned int) atoi(argv[1]);
    bool success = in.open(port);
    if (success) 
    {
        std::cout << "\nReading MIDI input from port " << port << "..." << std::endl;
        //char input;
        //std::cin.get(input);
        unsigned int duration = 10;
        std::cout << "Sleeping for " << duration << " seconds..." << std::endl;
        sleep(10);
    } else {
        std::cout << "Failed to open port " << port << std::endl;
    }
    return 0;
}

