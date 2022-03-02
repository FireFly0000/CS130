#include "application.h"

int main(int argc, char* argv[])
{
    int width = 640;
    int height = 480;

    application app;
    app.use_depth_buffer=false;
    app.use_perspective=false;
    app.init(argc, argv, width, height);
    app.run();
}
