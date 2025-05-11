#include "gui.h"
#include "Utils.h"

GUI guiApp;

int main(int argc, char *argv[])
{
    if(guiApp.init())
    {
        return 1;
    }

    if(argc >= 2)
    {
        if(guiApp.load(argv[1]))
        {
            return 1;
        }
    }

    return guiApp.main();
}