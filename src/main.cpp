#include "app.h"

int main(int argc, char *argv[])
{
    Simgui sg;
    if (sg.init())
        sg.run();
}