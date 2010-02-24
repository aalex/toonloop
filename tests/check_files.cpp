#include <iostream>
#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

int main(int argc, char *argv[])
{
    // create path
    fs::path example_path("/tmp/spam");
    fs::remove_all(example_path);
    fs::create_directories(example_path);
    // std::ofstream f = fs::file(example_path / "ham.txt");
    // f << "hello\n";
    // f.close();
    // if (! fs::exists(example_path / "ham.txt"))
    // {
    //     std::cout << "file does not exist.\n" << std::endl;
    // }
    return 0;
}


