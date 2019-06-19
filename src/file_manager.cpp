#include "file_manager.h"
#include <fstream>
#include <string>
#include <iostream>


simgui::file_manager::file_manager(std::string& file_name) : file_name(file_name)
{
    int pos = file_name.find_last_of('.');
    std::string name = file_name.substr(0, pos);
    std::string extension = file_name.substr(pos + 1); //get the file type
    file_extension = extension;
    
    //verify it's of the correct type
    if (file_extension == "c")
    {
        file_type = file_type::C;
    }
    else if (file_extension == "s")
    {
        file_type = file_type::S;
    }
    else if (file_extension == "asm")
    {
        file_type = file_type::ASM;
    }
    else 
    {
        throw new std::runtime_error(file_name + " doesn't appear to be a .c, .s, or .asm file\n");
    } 
    std::ifstream f(file_name.c_str()); //verify the file actually exists
    if (!(f.good()))
        throw new std::runtime_error(file_name + " couldn't be found.\n");
}

bool simgui::file_manager::compile(simgui::file_manager::compile_args c)
{
    std::string args = "-mmcu=";
    args += c.mmcu += " ";
    if ((c.type == simgui::file_manager::file_type::C))
    {
        args += "Wa,";
    }
    if (c.omit_false_conditionals)
    {
        args += "c";
    }
    if (c.omit_debugging_directives)
    {
        args += "d";
    }
    if (c.include_general_information)
    {
        args += "g";
    }
    if (c.include_highlevel_source)
    {
        args += "h";
    }
    if (c.include_assembly)
    {
        args += "l";
    }
    if (c.include_macro_expansions)
    {
        args += "m";
    }
    if (c.omit_forms_processing)
    {
        args += "n";
    }
    if (c.include_symbols)
    {
        args += "s";
    }
    if (c.keep_locals)
    {
        args +=",-L";
    }
    if (c.file_name != "")
    {
        args += ",=";
        args += c.file_name;
    } 
    else
    {
        args += ",=";
        args += file_name;
    }
    std::cout << args << "\n";
    return false;
}