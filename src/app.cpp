#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"
#include "sim_avr.h"
#include "file_manager.h"
#include "listing_file.h"
#include "subprocess.hpp"
#include "app.h"
#include "json.hpp"
#include <GL/gl3w.h>  
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

//default constructor
Simgui::Simgui() : arch_("avr6"), mmcu_("atmega2560"), config_(""), status_(Status::EMPTY), 
    avr_(nullptr), show_demo_window_(false), show_status_window_(true),  
    show_listing_window_(false), show_register_file_(false), show_flash_memory_(false), 
    show_data_memory_(false), show_peripherals_(false) {}


void Simgui::parse_config(std::string file_name)
{
    config_ = file_name;
    std::ifstream config_file(file_name.c_str());
    nlohmann::json config;
    config_file >> config;
    arch_ = config["arch"];
    mmcu_ = config["mmcu"];
    for (auto peripheral : config["peripherals"])
    {
        Peripheral p;
        p.num_ = peripheral["num"];
        p.name_ = peripheral["name"];
        for (auto input: peripheral["inputs"])
        {  
            InputPeripheral ip;
            ip.name_ = input["name"];
            for (auto mod: input["mod"])
            {
                //for some reason I have to convert it to string first or it fails
                int int_mem = std::stoul(std::string(mod["mem"]), nullptr, 16);
                int int_val = std::stoul(std::string(mod["val"]), nullptr, 16);
                ip.mods_[int_mem] = int_val;
            }
            p.inputs_.push_back(ip);
        }
        for (auto output: peripheral["outputs"])
        {
            OutputPeripheral op;
            op.type_ = output["type"];
            op.name_ = output["name"];
            if (op.type_ == "byte")
            {
                for (auto mem: output["mem"])
                {
                    //for some reason I have to convert it to string first or it fails
                    int int_mem = std::stoul(std::string(mem), nullptr, 16);
                    op.reads_.push_back(int_mem);
                }
            } 
            else if (op.type_ == "bit")
            {
                op.bit_mem_ = std::stoul(std::string(output["mem"]), nullptr, 16);
            } 
            else if (op.type_ == "ascii")
            {
                op.ascii_start_ = std::stoul(std::string(output["start"]), nullptr, 16);
                op.ascii_end_ = std::stoul(std::string(output["end"]), nullptr, 16);
            }
            
            p.outputs_.push_back(op);
        }
        peripherals_.push_back(p);
    }
}

void Simgui::start_simavr()
{
    std::string hex_file = fm_.get_hex_file();
    const char* c_hex_file = hex_file.c_str();
    const char* simavr_args[] = {"./simavr", "-f", "16000000", "-m", mmcu_.c_str(), "--dump-vitals", "-", c_hex_file};
    status_ = Status::SIMULATING;
    //calling this while avr points at something already is a memory leak I'm pretty sure
    avr_ = run_avr_main(8, const_cast<char**>(simavr_args)); 
}

void Simgui::sim()
{
    fm_.assemble(mmcu_); //assemble the assembly
    fm_.generate_hex(); //generate the ihex
    fm_.generate_lst(arch_); //generate the listing file
    lst_.set_source_file(fm_.get_lst_file().c_str());
    lst_.parse_lst(); //parse the listing file to generate the instruction list
    start_simavr(); //start simavr
}

void Simgui::stopsim()
{
    status_ = Status::READY;
    //if we don't close these windows first, the program will crash after I set avr_ to nullptr
    show_listing_window_ = false;
    show_register_file_ = false;
    show_flash_memory_ = false;
    show_data_memory_ = false;
    show_peripherals_ = false;
    avr_ = nullptr; //99.9999% sure this is a memory leak
}


//all this code is copied from the demo, I don't know what it does
bool Simgui::init()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;
    //if you update dear imgui you might have to change this version number
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window_ = glfwCreateWindow(1280, 720, "SIMGUI", NULL, NULL);
    if (window_ == NULL)
        return false;
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    bool err = gl3wInit() != 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return false;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    return true;
}

//a helper for creating menuitems with tooltips
bool MyMenuItem(const char* menu_text, const char* tooltip_text = "")
{
    bool b = ImGui::MenuItem(menu_text);
    if (tooltip_text != "")
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tooltip_text);
    return b;
}

void Simgui::run()
{
    while (!glfwWindowShouldClose(window_))
    {
        //this code was stolen from the demo, dunno what it does
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //todo make this an enum 
        static int execution_state = 0; //0 = still in progress, 1 = done, 2 = crashed
        //this is the menu bar that sits at the top of the main window
        if (ImGui::BeginMainMenuBar())
        {
            if (MyMenuItem("Load Assembly File", "Select an assembly file to simulate."))
            {
                auto zenity = subprocess::check_output(
                    {"zenity", "--file-selection", "--title=\"Select a file to assemble and run\""}, //get the file name
                    subprocess::error{"zenity_errors.txt", false});
                std::string s(zenity.buf.data());
                status_ = Status::READY;
                fm_.open(s.substr(0, s.length() - 1)); // s has a trailing newline
            }
            if (MyMenuItem("Load Config", "Load a config json file that specifies the architecture and any hardware peripherals you'd like to simulate."))
            {
                auto zenity = subprocess::check_output(
                    {"zenity", "--file-selection", "--title=\"Select a file to assemble and run\""}, //get the file name
                    subprocess::error{"zenity_errors.txt", false});
                std::string s(zenity.buf.data());
                std::cout << "parsing " << s << "\n";
                parse_config(s.substr(0, s.length() - 1)); // s has a trailing newline
            }
            if (status_ == Status::READY)
            {
                if (MyMenuItem("Begin Simulating","Assemble the currently loaded assembly file and start up Simavr. Uses the architecture currently described in the status window."))
                {
                    sim();
                }
            }
            if (status_ == Status::SIMULATING)
            {
                if (MyMenuItem("Execute Next", "Execute the next assembly instruction highlighted in pink."))
                {
                    if (execution_state == 0)
                    {
                        //go to sim_core.c/avr_run_one() to see what this actually does
                        int state = avr_run(avr_);
                        if (state == cpu_Done)
                            execution_state = 1;
                        if (state == cpu_Crashed)
                            execution_state = 2;
                    }
                }
                /*if (MyMenuItem("Execute Until Breakpoint", "Execute instructions until a breakpoint is reached, or the program terminates."))
                {
                    while (execution_state == 0 && !breakpoint_)
                    {
                        //go to simavr/sim/sim_core.c/avr_run_one() to see what this actually does
                        int state = avr_run(avr_);
                        if (state == cpu_Done)
                            execution_state = 1;
                        if (state == cpu_Crashed)
                            execution_state = 2;
                    }
                } this doesn't work yet */
                if (MyMenuItem("Stop Simulating","Unload the program from Simavr. (LIKELY CAUSES MEMORY LEAK)."))
                {
                    stopsim();
                }
                if (MyMenuItem("Restart Simulation","Unload then load the same program into Simavr. (LIKELY CAUSES MEMORY LEAK)."))
                {
                    sim();
                }
            }
            
            ImGui::EndMainMenuBar();
        }
        //use code blocks to separate windwows cleanly
        //no boolean for this window because the main window is always displayed
        {
            ImGui::Begin("This is the main window.");
            ImGui::Text("Use this window to select which other windows you would like to see.");
            ImGui::Checkbox("Show the demo window?", &show_demo_window_);
            ImGui::Checkbox("Show status window?", &show_status_window_);
            if (status_ == Status::SIMULATING)
            {
                ImGui::Checkbox("Show the listing file?", &show_listing_window_);
                ImGui::Checkbox("Show the register file?", &show_register_file_);
                ImGui::Checkbox("Show flash memory?", &show_flash_memory_);
                ImGui::Checkbox("Show data memory?", &show_data_memory_);
                if (config_ != "")
                    ImGui::Checkbox("Show peripherals?", &show_peripherals_);
                else
                    ImGui::Text("No config loaded - can't show peripherals.");
            }
            ImGui::End();
        }
        //the demo window basically has all the features in dear imgui.
        //I have it here so I can open it and find something I want,
        //then I ctrl+F the source code for words from the thing I want until
        //I figure out how they made it.
        if (show_demo_window_)
            ImGui::ShowDemoWindow(&show_demo_window_);

        if (show_status_window_)
        {
            ImGui::Begin("Status window");
            ImGui::Text("This window displays the status of your simulation.");
            ImGui::Text("Current architecture: %s", arch_.c_str());
            ImGui::Text("Current mmcu: %s", mmcu_.c_str());
            ImGui::Text("Current assembly file: %s", fm_.get_source_file().c_str());
            ImGui::Text("Current config file: %s", config_.c_str());
            switch(status_)
            {
                case Status::EMPTY: 
                    ImGui::Text("Status: waiting for assembly file.");
                    break;
                case Status::READY:
                    ImGui::Text("Status: ready to simulate.");
                    break;
                case Status::SIMULATING:
                    ImGui::Text("Status: simulating.");
                    break;
            }
            ImGui::End();
        }

        if (show_listing_window_)
        {
            static int test_type = 0;
            static ImGuiTextBuffer log;
            static int lines = lst_.size();
            bool breakpoints[lines] = {false};
            ImGui::Begin("Listing File");
            int pc = avr_->pc;

            ImGui::Columns(4, "lst");
            ImGui::Separator();
            ImGui::Text("Line"); ImGui::NextColumn();
            ImGui::Text("Mem"); ImGui::NextColumn();
            ImGui::Text("Opcode"); ImGui::NextColumn();
            ImGui::Text("Instr"); ImGui::NextColumn();
            ImGui::Separator();
            for (int i = 0; i < lst_.size(); i++)
            {
                listing_file::instruction instr_to_print = lst_[i];
                //want to print like "1 0002 0AE9 ldi r16, 1"
                int byte_address = instr_to_print.byte_address_;
                char line_buff[5];
                char mem_buff[8];
                char opcode_buff[10];
                char instr_buff[100];
                sprintf(line_buff, "%3i", instr_to_print.line_);
                sprintf(mem_buff, "%04hhx", instr_to_print.byte_address_>>1);
                sprintf(opcode_buff, "%s%c", instr_to_print.opcode_.c_str(), '\0');
                sprintf(instr_buff, "%s", instr_to_print.instr_.c_str());
                //highlight the instruction to be executed by checking the program counter
                if (byte_address == pc)
                {
                    
                    ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), "%s", line_buff); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), "%s", mem_buff); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), "%s", opcode_buff); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), "%s", instr_buff); ImGui::NextColumn();
                }
                else
                {
                    ImGui::TextUnformatted(line_buff); ImGui::NextColumn();
                    ImGui::TextUnformatted(mem_buff); ImGui::NextColumn();
                    ImGui::TextUnformatted(opcode_buff); ImGui::NextColumn();
                    ImGui::TextUnformatted(instr_buff); ImGui::NextColumn();
                }
                ImGui::Separator();
            }  
            
            ImGui::End();
                    
        }
        if (show_register_file_)
        {
            ImGui::Begin("Register File");
            ImGui::Text("Stack Pointer: %02hhx%02hhx", avr_->data[R_SPH], avr_->data[R_SPL]);
            ImGui::Text("X Pointer: %02hhx%02hhx", avr_->data[R_XH], avr_->data[R_XL]);
            ImGui::Text("Y Pointer: %02hhx%02hhx", avr_->data[R_YH], avr_->data[R_YL]);
            ImGui::Text("Z Pointer: %02hhx%02hhx", avr_->data[R_ZH], avr_->data[R_ZL]);
            ImGui::Text("Program Counter: %04hhx\n", (avr_->pc)>>1);
            ImGui::Text("Status Register");
            ImGui::Columns(8, "sreg");
            ImGui::Separator();
            ImGui::Text("I"); ImGui::NextColumn();
            ImGui::Text("T"); ImGui::NextColumn();
            ImGui::Text("H"); ImGui::NextColumn();
            ImGui::Text("S"); ImGui::NextColumn();
            ImGui::Text("V"); ImGui::NextColumn();
            ImGui::Text("N"); ImGui::NextColumn();
            ImGui::Text("Z"); ImGui::NextColumn();
            ImGui::Text("C"); ImGui::NextColumn();
            
            for (int i = 0; i < 8; i++)
            {
                ImGui::Text("%d", avr_->sreg[i]); 
                ImGui::NextColumn();
            }
            ImGui::Separator();
            
            //hardcoding 32 registers here
            ImGui::Columns(4, "rfile");
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    int reg = j + i*4;
                    ImGui::Text("R%d: %02hhx\n", reg, avr_->data[reg]);
                    ImGui::NextColumn();
                }
            }  
            ImGui::Columns(1);
            ImGui::End(); 
        }
        if (show_flash_memory_)
        {
            //might want to modify this to not be editable - flash memory should be read only
            flash_mem_.DrawWindow("Flash Memory", avr_->flash, 1024, 0x0000);
        }
        if (show_data_memory_)
        {
            //this will need to be modified if you want to stack memory
            //probably should be a parameter in the configuration files
            data_mem_.DrawWindow("Data Memory", avr_->data, 1024, 0x0000);
        }

        //display peripherals from the config
        if (show_peripherals_)
        {
            for (Peripheral p: peripherals_)
            {

                ImGui::Begin(p.name_.c_str());
                for (InputPeripheral input : p.inputs_)
                {
                    ImGui::SameLine();
                    if (ImGui::Button(input.name_.c_str()))
                    {
                        for (auto pair: input.mods_)
                        {
                            avr_->data[pair.first] = pair.second;
                        }
                    }
                }
                for (OutputPeripheral output : p.outputs_)
                {
                    ImGui::Text("%s\n", output.name_.c_str());
                    if (output.type_ == "bit")
                    {
                        ImGui::SameLine();
                        int val = std::pow(2, output.position_);
                        ImGui::Text("(0x%04x): %d\n", output.bit_mem_, avr_->data[output.bit_mem_] & val);
                    } 
                    else if (output.type_ == "byte")
                    {
                        for (int mem : output.reads_)
                        {
                            ImGui::SameLine();
                            ImGui::Text("Mem 0x%hhx: 0x%02x\n", mem, avr_->data[mem]);
                        }
                    }
                    else if (output.type_ == "ascii")
                    {
                        std::string ascii;
                        for (int i = output.ascii_start_; i < output.ascii_end_; i++)
                        {
                            ascii += avr_->data[i];
                        }
                        ImGui::Text("%s", ascii.c_str());
                    }   
                }
                ImGui::End();
            }
        }

        //this is stuff from the demo, I don't know exactly what it's doing
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window_);
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwMakeContextCurrent(window_);
        glfwSwapBuffers(window_);
    }
}

