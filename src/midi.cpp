//*****************************************//
//  cmidiin.cpp
//  by Gary Scavone, 2003-2004.
//
//  Simple program to test MIDI input and
//  use of a user callback function.
//
//*****************************************//

#include <iostream>
#include <cstdlib>
#include "stk/RtMidi.h"

void usage( void ) {
    // Error function in case of incorrect command-line
    // argument specifications.
    std::cout << "\nuseage: cmidiin <port>\n";
    std::cout << "    where port = the device to use (default = 0).\n\n";
    exit( 0 );
}

void mycallback( double deltatime, std::vector< unsigned char > *message, void *userData )
{
    unsigned int nBytes = message->size();
    for ( unsigned int i=0; i<nBytes; i++ )
        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    if ( nBytes > 0 )
        std::cout << "stamp = " << deltatime << std::endl;
}

int main( int argc, char *argv[] )
{
    RtMidiIn *midiin = 0;
  
    // Minimal command-line check.
    if (argc > 2) 
        usage();
  
    // RtMidiIn constructor
    try {
        midiin = new RtMidiIn();
    }
    catch (RtError &error) {
        error.printMessage();
        exit( EXIT_FAILURE );
    }
  
    // Check available ports vs. specified.
    unsigned int port = 0;
    unsigned int nPorts = midiin->getPortCount();

    // List inputs.
    std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
    std::string portName;
    for (unsigned int i=0; i < nPorts; i++) 
    {
        try {
            portName = midiin->getPortName(i);
        }
        catch (RtError &name_error) {
            name_error.printMessage();
            goto cleanup; // FIXME no goto
        }
        std::cout << "  Input Port #" << i << ": " << portName << '\n';
    }

    if (argc == 2) 
        port = (unsigned int) atoi(argv[1]);
    if (port >= nPorts) 
    {
        delete midiin;
        std::cout << "Invalid port specifier!\n";
        usage();
    }
  
    try 
    {
        midiin->openPort(port);
    }
    catch (RtError &error) 
    {
        error.printMessage();
        goto cleanup; // FIXME: no goto
    }
  
    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue instead of sent to the callback function.
    midiin->setCallback(&mycallback);
  
    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes(false, false, false);
  
    std::cout << "\nReading MIDI input from port " << port << "... press <enter> to quit.\n";
    char input;
    std::cin.get(input);
  
    // Clean up
    cleanup: // FIXME: no goto
        delete midiin;
  
    return 0;
}

