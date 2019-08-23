#ifndef SIMGUI_H
#define SIMGUI_H

#include "imgui.h"
#include "file_manager.h"
#include "listing_file.h"
#include "sim_avr.h"
#include <GL/gl3w.h>  
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <map>
class Simgui
{
    public:


        enum class Status 
        {
            EMPTY, //as in no file has been loaded
            READY, //as in there is a file that has been loaded
            SIMULATING //as in there is an active simulation
        };

        struct InputPeripheral
        {
            std::string name_;
            std::map<int, int> mods_;
        };

        struct OutputPeripheral
        {
            std::string name_;
            std::vector<int> reads_;
        };

        struct Peripheral //each peripheral corresponds to a window
        {

            int num_; // how many "things" there will be in the window
            std::string name_; // what to call the window
            std::vector<InputPeripheral> inputs_; //the input things in the window
            std::vector<OutputPeripheral> outputs_; //the output things in the window
        };


    private:
        
        std::string arch_ = "avr6";
        std::string mmcu_ = "atmega2560";
        std::vector<Peripheral> peripherals_;
        GLFWwindow* window_;
        file_manager fm_; 
        listing_file lst_;
        avr_t* avr_ = nullptr;
        Status status_ = Status::EMPTY;

    public:

        bool init();
        void run();
        void sim();
        void parse_config(std::string file_name);
};

#endif