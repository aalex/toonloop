#include <iostream>
#include "boost/filesystem.hpp"

int main(int argc, char **argv)
{
    namespace fs = boost::filesystem;

    fs::path directory = fs::path("/tmp/hellooo");
    fs::path to_p = fs::path("/etc/apt/sources.list");
    fs::path from_p = fs::path("/tmp/hellooo/sucess");

    if (!fs::exists(directory)) 
    { 
        std::cout << "creating directory" << std::endl;
        bool success = fs::create_directory(directory);
        if (!success)
            std::cout << "failed to create directory" << std::endl;
    }

    if (!fs::exists(to_p)) 
    { 
        std::cout << "Target doesn't exist!" << std::endl;
    } else if (fs::exists(from_p)) {
        std::cout << "Symlink already exists!" << std::endl;
    } else {
        //int error_code = 
        try
        {
            std::cout << "creating symlink" << std::endl;
            fs::create_symlink(to_p, from_p);
        } catch(const fs::filesystem_error &e) {
            std::cerr << "Error creating symlink: " << e.what() << std::endl;
        }
        //if (error_code != 0)
        //    std::cout << "ERROR!" << std::endl;
        std::cout << "Success!" << std::endl;
    }
    return 0;
}
