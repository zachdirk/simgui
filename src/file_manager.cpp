#include "file_manager.h"
#include <fstream>
#include <string>
#include <iostream>
#include "subprocess.hpp"
std::string file_manager::get_file_name_no_extension(const std::string& file_name)
{
    return file_name.substr(0, file_name.find_last_of('.'));
}

file_manager::file_type file_manager::get_file_type(const std::string& file_name)
{
    std::string extension = file_name.substr(file_name.find_last_of('.') + 1); //get the file extension
    if (extension == "c")
        return file_type::C;
    else if (extension == "S")
        return file_type::S;
    else if (extension == "asm")
        return file_type::ASM;
    else
        return file_type::OTHER;
    
}

bool file_manager::file_exists(const std::string& file_name)
{
    std::ifstream f(file_name.c_str()); //verify the file actually exists
    return f.good();
}

file_manager::file_manager(std::string& file_name) :good(true) //assume it succeeds until it fails
{
    cf.source_file = file_name;
    cf.no_extension = get_file_name_no_extension(file_name);
    file_type extension = get_file_type(file_name);
    if (extension == file_type::C)
    {
        cf.type = file_type::C;
        cf.assembly_file = cf.no_extension + ".S";
    }
    else if (extension == file_type::S)
    {
        cf.type = file_type::S;
        cf.assembly_file = file_name;
    }
    else if (extension == file_type::ASM)
    {
        cf.type = file_type::ASM;
        cf.assembly_file = file_name;
    }
    else 
    {
        std::cerr << file_name << "doesn't appear to be a .c, .S, or .asm file\n";
        good = false;
    }
    //at this point we know it's a valid file type
    cf.listing_file = cf.no_extension + ".lst";
    cf.object_file = cf.no_extension + ".o";
    cf.hex_file = cf.no_extension + ".hex";
    if (!file_exists(file_name.c_str())) //verify the file actually exists
    {
        std::cerr << file_name << " couldn't be found.\n";
        good = false;
    }

}

bool file_manager::assemble()
{
    //hard code the assembly options for now, see readme for explanations
    auto assemble = subprocess::Popen(
        {"avr-gcc", "-nostartfiles", "-mmcu=atmega2560", "-x", "assembler-with-cpp", "-g", cf.assembly_file.c_str(), "-o", cf.object_file.c_str()}, //command and args
        subprocess::error{"assembler_errors.txt"}); //redirect errors if there's a problem assembling
    assemble.wait(); //wait for assembly to finish
    return true; //I'm assuming this will return 
}


bool file_manager::generate_hex()
{
    //todo
    auto generate_hex = subprocess::Popen(
        {"avr-objcopy", "-0", "ihex", cf.object_file.c_str(), cf.hex_file.c_str()}, //command + args
        subprocess::error{"generate_hex_errors.txt"}); //redirect errors if there's a problem generating the hex
    generate_hex.wait(); //wait for generation to finish
    return true;
}

bool file_manager::generate_lst()
{
    auto generate_lst = subprocess::Popen(
        {"avr-objdump", "-m", "avr6", "-S", "-l", "-d", "-g", cf.object_file.c_str()}, //specify command and arguments
        subprocess::output{cf.listing_file.c_str()}, //redirect output 
        subprocess::error{"lst_errors.txt"}); //redirect errors if there's a problem generating the lst file
    generate_lst.wait(); //wait for generation to finish
    return true; //I'm assuming this will return 
}

bool file_manager::compile(const file_manager::compile_args& c)
{
    //assume file type is C
    std::string args = "-mmcu=";
    args += c.mmcu ;
    args += " ";
    if ((c.type == file_manager::file_type::C))
    {
        args += "-Wa,-a";
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
        args +=",-L ";
    }
    if (c.debug)
    {
        args += "-g ";
    }
    if (c.save_temps)
    {
        args += "-save-temps ";
    }
    if (c.verbose_asm)
    {
        args += "-fverbose-asm ";
    }
    args += cf.source_file;
    args += " -o ";
    args += cf.object_file;
    args += " > ";
    if (c.file_name != "")
    {
        args += c.file_name;
    } 
    else
    {
        args += cf.listing_file;
    }
    std::string command = "avr-gcc " + args;
    int err = system(command.c_str());
    return err != 0;
}