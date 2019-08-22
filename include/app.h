#ifndef SIMGUI_H
#define SIMGUI_H

#include "imgui.h"
#include "file_manager.h"
#include "listing_file.h"
#include "sim_avr.h"
#include <GL/gl3w.h>  
#include <GLFW/glfw3.h>
class Simgui
{
    public:

        enum class Status 
        {
            EMPTY, //as in no file has been loaded
            READY, //as in there is a file that has been loaded
            SIMULATING //as in there is an active simulation
        };


    private:
        
        GLFWwindow* window_;
        file_manager fm_; 
        listing_file lst_;
        avr_t* avr_ = nullptr;
        Status status_ = Status::EMPTY;

    public:

        bool init();
        void run();
        void sim();
};

#endif