
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"
#include "sim_avr.h"
#include "file_manager.h"
#include "listing_file.h"
#include <GL/gl3w.h>  
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string>
#include <iostream>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int argc, char *argv[])
{
    // Setup window
    if (argc < 2)
    {
        std::cerr << "Need at least a file name to compile.\n";
        return(1);
    }
    std::string s(argv[1]);
    file_manager fm(s);
    if (!(fm.is_good()))
    {
        std::cerr << "There was a problem with the file you provided.\n";
        return(1);
    }
    file_manager::compile_args c;
    /* figure this part out later
    if (fm.compilation_file_type == file_manager::C)
    {
        fm.compile(c);
    }*/
    fm.assemble();
    fm.generate_hex();
    fm.generate_lst();
    listing_file lst(fm.get_lst_file());
    auto i = lst.parse_lst();
    const char* hex_file = fm.get_hex_file().c_str();
    const char* simavr_args[] = {"./simavr", "-f", "16000000", "-m", "atmega2560", "--dump-vitals", "-", hex_file};
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    bool err = gl3wInit() != 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    bool show_demo_window = false;
    bool show_leds = false;
    bool show_buttons = false;
    bool show_shield = false;
    bool show_listing_window = false;
    bool show_register_file = false;
    bool show_flash_memory = false;
    bool show_data_memory = false;
    avr_t* avr = run_avr_main(8, const_cast<char**>(simavr_args));
    static MemoryEditor flash_mem; 
    static MemoryEditor data_mem; 
    while (!glfwWindowShouldClose(window))
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
        {
            ImGui::Begin("This is the main window.");
            ImGui::Text("Use this window to select which other windows you would like to see.");
            ImGui::Checkbox("Show the demo window?", &show_demo_window);
            ImGui::Checkbox("Show the listing file?", &show_listing_window);
            ImGui::Checkbox("Show the register file?", &show_register_file);
            ImGui::Checkbox("Show flash memory?", &show_flash_memory);
            ImGui::Checkbox("Show data memory?", &show_data_memory);
            ImGui::Checkbox("Show leds memory?", &show_leds);
            ImGui::End();
        }
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        if (show_listing_window)
        {
            static int test_type = 0;
            static ImGuiTextBuffer log;
            static int lines = lst.size();
            ImGui::Begin("Listing File");
            int pc = avr->pc;
            ImGui::Text("Current Program Counter: %04hhx\n", pc>>1);
            ImGui::SameLine();
            if (ImGui::Button("Execute next line"))
            {
                if (execution_state == 0)
                {
                    //go to sim_core.c/avr_run_one() to see what this actually does
                    int state = avr_run(avr);
                    if (state == cpu_Done)
                        execution_state = 1;
                    if (state == cpu_Crashed)
                        execution_state = 2;
                }
            }
            ImGuiListClipper clipper(lines);
            ImGui::Columns(4, "lst");
            ImGui::Separator();
            ImGui::Text("Line"); ImGui::NextColumn();
            ImGui::Text("Mem"); ImGui::NextColumn();
            ImGui::Text("Opcode"); ImGui::NextColumn();
            ImGui::Text("Instr"); ImGui::NextColumn();
            ImGui::Separator();
            while (clipper.Step())
            {
                
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                {
                    listing_file::instruction instr_to_print = lst[i];
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
                ImGui::Text("%d", avr->sreg[i]); 
                ImGui::NextColumn();
            }
            ImGui::Separator();
            ImGui::Columns(4, "rfile");
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    int reg = j + i*4;
                    ImGui::Text("R%d: %02hhx\n", reg, avr->data[reg]);
                    ImGui::NextColumn();
                }
            }  
            ImGui::Columns(1);
            ImGui::End(); 
        }
        if (show_flash_memory)
        {
            flash_mem.DrawWindow("Flash Memory", avr->flash, 1024, 0x0000);
        }
        if (show_data_memory)
        {
            data_mem.DrawWindow("Data Memory", avr->data, 1024, 0x0000);
        }
        if (show_leds)
        {
            uint8_t PORTL = avr->data[0x10B];
            uint8_t PORTB = avr->data[0x25];
            ImGui::Begin("LEDs");
            ImGui::Text("LED Display");
            ImGui::Columns(6, "leds");
            ImGui::Separator();
            if (PORTL & 0b10000000)
            {
                ImGui::Text("1"); ImGui::NextColumn();
            }  
            else 
            {
                ImGui::Text("0"); ImGui::NextColumn();
            }
               
            if (PORTL & 0b00100000)
            {
                ImGui::Text("1"); ImGui::NextColumn();
            }
            else 
            {
                ImGui::Text("0"); ImGui::NextColumn();
            } 
            if (PORTL & 0b00001000)
            {
                ImGui::Text("1"); ImGui::NextColumn();
            }
            else 
            {
                ImGui::Text("0"); ImGui::NextColumn();
            }
                
            if (PORTL & 0b00000010)
            {
                ImGui::Text("1"); ImGui::NextColumn();  
            }
                
            else 
            {
                ImGui::Text("0"); ImGui::NextColumn();
            }
            if (PORTB & 0b00001000)
            {
                ImGui::Text("1"); ImGui::NextColumn();
            }
            else 
            {
                ImGui::Text("0"); ImGui::NextColumn();
            }
            if (PORTB & 0b00000010)
            {
                ImGui::Text("1"); ImGui::NextColumn();
            }
            else 
            {
                ImGui::Text("0"); ImGui::NextColumn();
            }

            ImGui::End();
        }


        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
}