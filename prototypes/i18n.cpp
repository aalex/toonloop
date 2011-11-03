#include <libintl.h> // gettext
#include <iostream>

bool checkGettext()
{
    std::cout << gettext("Hello world.");
    return true;
}

int main(int argc, char *argv[])
{
    if (! checkGettext())
        return 1;
    return 0;
}

