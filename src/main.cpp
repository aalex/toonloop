#include "application.h"
#include <iostream>
// gint gint gchar
int main(int argc, char* argv[])
{

    try 
    {
        Application::get_instance().run(argc, argv);
    }
    catch(const std::exception& e) 
    {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch (...) 
    {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }
    return 0;
}
