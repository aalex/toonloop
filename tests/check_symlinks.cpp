#include <iostream>
#include "boost/filesystem.hpp"

int main(int argc, char **argv)
{
    using namespace boost::filesystem;

    path directory = path("/tmp/hellooo");
    path to_p = path("/etc/apt/sources.list");
    path from_p = path("/tmp/hellooo/sucess");

    if (!exists(directory)) 
    { 
        bool create_directory(&directory);
    }

    if (!exists(to_p)) 
    { 
        std::cout << "Target doesn't exist!" << std::endl;
    } else if (exists(from_p)) {
        std::cout << "Symlink already exists!" << std::endl;
    } else {
        //int error_code = 
        create_symlink(to_p, from_p);
        //if (error_code != 0)
        //    std::cout << "ERROR!" << std::endl;
        std::cout << "Success!" << std::endl;
    }
    return 0;
}
