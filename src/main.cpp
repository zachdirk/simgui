#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "sim_avr.h"
#include "file_manager.h"
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
        std::cout << "Need at least a file name to compile.\n";
        return(1);
    }
    std::string s(argv[1]);
    file_manager fm(s);

    /* 
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

    bool show_leds = false;
    bool show_buttons = false;
    bool show_shield = false;
    bool show_code_window = false;
    avr_t* avr = run_avr_main(argc, argv);
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
        static bool leds[6];
        //main window
        {
            ImGui::Begin("This is the main window.");
            ImGui::Text("Use this window to select which other windows you would like to see.");
            ImGui::Checkbox("Show the LED window?", &show_leds);
            ImGui::Checkbox("Show the buttons window?", &show_buttons);
            ImGui::Checkbox("Show the LCD shield window?", &show_shield);
            ImGui::Checkbox("Show the code window?", &show_code_window);
            ImGui::End();
        }
        if (show_leds)
        {
            
            ImGui::Begin("This window shows the current values of the LEDs.");
            std::string str;
            for (int i = 0; i < 6; i++)
            {
                if (leds[i])
                    str = "Led " + std::to_string(i) + " is on";
                else
                    str = "Led " + std::to_string(i) + " is off";
                ImGui::Text("%s",str.c_str());
            }
            ImGui::End();
        }
        if (show_buttons)
        {
            ImGui::Begin("This is the button window!");
            if (ImGui::Button("Up"))
                leds[0] = !leds[0];
            if (ImGui::Button("Select"))
                leds[1] = !leds[1];
            ImGui::SameLine();
            if (ImGui::Button("Left"))
                leds[2] = !leds[2];
            
            
            if (ImGui::Button("Right"))
                leds[3] = !leds[3];
            if (ImGui::Button("Reset"))
                leds[4] = !leds[4];
            ImGui::SameLine();
            if (ImGui::Button("Down"))
                leds[5] = !leds[5];
            ImGui::End();
        }
        if (show_shield)
        {
            static int count = 0;
            ImGui::Begin("This is the LCD window!");
            char str1[] = "This is a very long string on line one!";
            char str2[] = "This line two string is shorter.";
            int len1 = strlen(str1);
            static int ptr1 = 0;
            int len2 = strlen(str2);
            static int ptr2 = 0;
            static char str1cpy[17];
            static char str2cpy[17];
            str1cpy[16] = '\0';
            str2cpy[16] = '\0';
            
            for (int i = 0; i < 16; i++)
            {
                str1cpy[i] = str1[(i+ptr1) % len1];
                str2cpy[i] = str2[(i+ptr2) % len2];   
            }
            if (count % 125 == 0)
            {
                ++ptr1;
                ++ptr2;
            }

            if (ptr1 == len1)
                ptr1 = 0;
            if (ptr2 == len2)
                ptr2 = 0;
            ImGui::Text("ptr1: %d ptr2: %d", ptr1, ptr2);
            ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), "%s", str1cpy);
            ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), "%s", str2cpy);
            ImGui::End();
            ++count;
        }
        if (show_code_window)
        {
            ImGui::Begin("This is the code window!");
            if (ImGui::Button("Execute"))
            {
                int state = avr_run(avr);
		        if (state == cpu_Done || state == cpu_Crashed)
			        avr_terminate(avr);
            }
            for (int i = 0; i < 10; i++)
            {
                ImGui::Text("%.4x",opcodes[i*10]);
                for (int j = 1; j < 10; j++)
                {
                    ImGui::SameLine();
                    ImGui::Text("%.4x",opcodes[i*10+j]);
                }
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
    */
}