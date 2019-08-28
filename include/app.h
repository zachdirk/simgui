#ifndef SIMGUI_H
#define SIMGUI_H

#include "imgui.h"
#include "file_manager.h"
#include "listing_file.h"
#include "sim_avr.h"
#include "imgui_memory_editor.h"
#include <GL/gl3w.h>  
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <map>

//a big fat class for holding all my information so it's not just sitting in global scope (even though it kinda is)
class Simgui
{
    public:

        //this is the enum for the simulation status
        enum class Status 
        {
            EMPTY, //as in no file has been loaded
            READY, //as in there is a file that has been loaded
            SIMULATING //as in there is an active simulation
        };

        //input peripherals correspond to peripherals that might change memory
        struct InputPeripheral
        {
            std::string name_; //name of the peripheral
            std::map<int, int> mods_; //map of memory addresses to values to change when this peripheral is clicked
        };

        //output peripherals correspond to peripherals that read memory
        struct OutputPeripheral
        {
            std::string name_; //name of the peripheral
            std::string type_; //type of the peripheral ("bit", "byte", "ascii")
            //used for byte types
            std::vector<int> reads_; //list of memory addresses to read bytes from
            //only used for bit types
            int bit_mem_; //the memory address containing the value 
            int position_; //which bit from the address to read
            //used for ascii types
            int ascii_start_; //the start of the ascii string in data memory
            int ascii_end_; //one past the end of the ascii string in data memory

        };

        //a peripheral is just a class that accesses memory via sub-peripherals
        //there's probably an easy way to use inheritance here but this is easier. 
        struct Peripheral //each peripheral corresponds to a window
        {

            int num_; // how many "things" there will be in the window
            std::string name_; // what to call the window
            std::vector<InputPeripheral> inputs_; //the input things in the window
            std::vector<OutputPeripheral> outputs_; //the output things in the window
        };


    private:
        
        std::string arch_; //architecture for compilation and simavr defaults to avr6
        std::string mmcu_; //mmcu (more specific architecture) for compilation defaults to mmcu
        std::vector<Peripheral> peripherals_; //list of peripherals from configuration files
        
        
        GLFWwindow* window_; //pointer to the main window from glfw
        file_manager fm_; //file manager which controls file names and assembling
        listing_file lst_; //listing file which controls decompiling and parsing 
        avr_t* avr_ = nullptr; //a pointer to the avr struct simavr uses to hold all the important stuff
        Status status_; //simulation status defaults to EMPTY
        std::string config_; //config file name

        //bools to handle the windows
        bool show_demo_window_;
        bool show_status_window_;
        bool show_listing_window_;
        bool show_register_file_;
        bool show_flash_memory_;
        bool show_data_memory_;
        bool show_peripherals_;
        //bool breakpoint_; haven't figured out breakpoints quite yet

        //memory editors
        MemoryEditor flash_mem_; 
        MemoryEditor data_mem_; 

    public:

        Simgui(); //default constructor, initializes values
        bool init(); //handles all the glfw init stuff, stolen mostly from the demo code
        void run(); //this is the main drawing loop, any code related to what you see is here
        void sim(); //assembles and generates the hex for simavr
        void stopsim(); //leaks memory and also stops simavr
        void restartsim(); //leaks memory and also restarts simavr
        void start_simavr(); //takes the assembled hex and starts simavr
        void parse_config(std::string file_name); //parses a json file to create the peripherals
};

#endif