#include <iostream>
#include "tween.h"

#define VERBOSE true

int main(int argc, char *argv[])
{
    int num_chases = 3;
    Tween chase[3];
    float res;
    float targets[3] ={ 1, 0, 1};
    
    for (int i = 0; i < num_chases; i++)
    {
         chase[i] = Tween();
         chase[i].setType((i < 1) ? TWEEN_LINEAR : TWEEN_EASEINOUTCUBIC);
         chase[i].line(targets[i], 100);
    }
    // print 100 interpolated values
    for (int t=0; t <= 100;t++)
    {
        if (VERBOSE)
            std::cout << "t:" << t << std::endl;
        for (int i=0;i<num_chases;i++)
        {
            // half way
            if (t == 50)
            {
                chase[0].line(0.4, 0); // STOP
                chase[1].line(0.25,100);
            }
            // Tween_tick() should be renamed
            res = chase[i].tick((float) t);
            if (VERBOSE)
                std::cout << " " << int(res * 255.0) << std::endl;
        }
    }
    return 0;
}

