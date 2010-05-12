#include <string>
#include "subprocess.h"

int main(int argc, char *argv[])
{
    std::string command("echo 'Hello'; sleep 1; echo 'bye'");
    if (run_command(command))
    {
        return 0;
    } else {
        return 1;
    }
}
