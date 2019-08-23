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

void Simgui::parse_config(std::string file_name)
{
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
                std::string mem = mod["mem"];
                int int_mem = std::stoul(mem, nullptr, 16);
                std::string val = mod["val"];
                int int_val = std::stoul(val, nullptr, 16);
                ip.mods_[int_mem] = int_val;
            }
            p.inputs_.push_back(ip);
        }
        for (auto output: peripheral["outputs"])
        {
            OutputPeripheral op;
            op.name_ = output["name"];
            for (auto read: output["read"])
            {
                std::string mem = read;
                int int_mem = std::stoul(mem, nullptr, 16);
                op.reads_.push_back(int_mem);
            }
            p.outputs_.push_back(op);
        }
        peripherals_.push_back(p);
    }
}

void Simgui::sim()
{
    printf("assembling %s\n", fm_.get_source_file().c_str());
    fm_.assemble(); //assembly the assembly
    printf("generating hex from %s\n", fm_.get_object_file().c_str());
    fm_.generate_hex(); //generate the ihex
    printf("generating lst from %s\n", fm_.get_object_file().c_str());
    fm_.generate_lst(); //generate the listing file
    printf("parsing %s\n", fm_.get_lst_file().c_str());
    lst_.set_source_file(fm_.get_lst_file().c_str());
    lst_.parse_lst();
    std::string hex_file = fm_.get_hex_file();
    const char* c_hex_file = hex_file.c_str();
    const char* simavr_args[] = {"./simavr", "-f", "16000000", "-m", "atmega2560", "--dump-vitals", "-", c_hex_file};
    avr_ = run_avr_main(8, const_cast<char**>(simavr_args));
}

bool Simgui::init()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;
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

void Simgui::run()
{
    bool show_demo_window = false;
    bool show_listing_window = false;
    bool show_register_file = false;
    bool show_flash_memory = false;
    bool show_data_memory = false;
    bool show_peripherals = false;
    static MemoryEditor flash_mem; 
    static MemoryEditor data_mem; 
    while (!glfwWindowShouldClose(window_))
    {
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static int execution_state = 0; //0 = still in progress, 1 = done, 2 = crashed
        //main window
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::MenuItem("Open"))
            {
                auto zenity = subprocess::check_output(
                    {"zenity", "--file-selection", "--title=\"Select a file to assemble and run\""}, //get the file name
                    subprocess::error{"zenity_errors.txt", false});
                std::string s(zenity.buf.data());
                fm_.open(s.substr(0, s.length() - 1)); // s has a trailing newline
            }
            if (ImGui::MenuItem("Begin Simulating"))
            {
                sim();
            }
            if (ImGui::MenuItem("Load Config"))
            {
                auto zenity = subprocess::check_output(
                    {"zenity", "--file-selection", "--title=\"Select a file to assemble and run\""}, //get the file name
                    subprocess::error{"zenity_errors.txt", false});
                std::string s(zenity.buf.data());
                std::cout << "parsing " << s << "\n";
                parse_config(s.substr(0, s.length() - 1)); // s has a trailing newline
            }
            ImGui::EndMainMenuBar();
        }
        {
            ImGui::Begin("This is the main window.");
            ImGui::Text("Use this window to select which other windows you would like to see.");
            ImGui::Checkbox("Show the demo window?", &show_demo_window);
            ImGui::Checkbox("Show the listing file?", &show_listing_window);
            ImGui::Checkbox("Show the register file?", &show_register_file);
            ImGui::Checkbox("Show flash memory?", &show_flash_memory);
            ImGui::Checkbox("Show data memory?", &show_data_memory);
            ImGui::Checkbox("Show peripherals?", &show_peripherals);
            ImGui::End();
        }
        


        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        if (show_listing_window)
        {
            static int test_type = 0;
            static ImGuiTextBuffer log;
            static int lines = lst_.size();
            ImGui::Begin("Listing File");
            int pc = avr_->pc;
            ImGui::Text("Current Program Counter: %04hhx\n", pc>>1);
            ImGui::SameLine();
            if (ImGui::Button("Execute next line"))
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
                sprintf(opcode_buff, "%s\0", instr_to_print.opcode_.c_str());
                sprintf(instr_buff, "%s", instr_to_print.instr_.c_str());
                if (byte_address == pc)
                {
                    ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), line_buff); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), mem_buff); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), opcode_buff); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), instr_buff); ImGui::NextColumn();
                    
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
        if (show_register_file)
        {
            ImGui::Begin("Register File");
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
        if (show_flash_memory)
        {
            flash_mem.DrawWindow("Flash Memory", avr_->flash, 1024, 0x0000);
        }
        if (show_data_memory)
        {
            data_mem.DrawWindow("Data Memory", avr_->data, 1024, 0x0000);
        }
        if (show_peripherals)
        {
            for (Peripheral p: peripherals_)
            {

                ImGui::Begin(p.name_.c_str());
                for (InputPeripheral input : p.inputs_)
                {
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
                    ImGui::Text(output.name_.c_str());
                    for (int mem : output.reads_)
                    {
                        ImGui::Text("Mem %02hhx: %02hhx\n", mem, avr_->data[mem]);
                    }
                }
                ImGui::End();
            }
        }
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

