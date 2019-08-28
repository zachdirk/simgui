#include "file_manager.h"
#include <fstream>
#include <string>
#include <iostream>
#include "subprocess.hpp"

std::string file_manager::get_file_name_no_extension(const std::string& file_name)
{
    return file_name.substr(0, file_name.find_last_of('.'));
}

void file_manager::open(std::string file_name)
{
    cf_.source_file_ = file_name;
    cf_.no_extension_ = get_file_name_no_extension(cf_.source_file_);
    cf_.listing_file_ = cf_.no_extension_ + ".lst";
    cf_.object_file_ = cf_.no_extension_ + ".o";
    cf_.hex_file_ = cf_.no_extension_ + ".hex";
    //debugging stuff printf("source file: \"%s\"\nobject file: \"%s\"\nlisting file: \"%s\"\nhex file: \"%s\"\n", cf_.source_file_.c_str(), cf_.object_file_.c_str(), cf_.listing_file_.c_str(), cf_.hex_file_.c_str());
}

void file_manager::assemble(std::string mmcu)
{
    //hard code the assembly options for now, see readme for explanations
    std::string mmcu_flag = "-mmcu=" + mmcu;
    auto assemble = subprocess::Popen(
        {"avr-gcc", "-nostartfiles", mmcu_flag.c_str(), "-x", "assembler-with-cpp", "-g", cf_.source_file_.c_str(), "-o", cf_.object_file_.c_str()}, //command and args
        subprocess::error{"assembler_errors.txt", false}); //redirect errors if there's a problem assembling
    assemble.wait(); //wait for assembly to finish
}


void file_manager::generate_hex()
{
    //todo
    auto generate_hex = subprocess::Popen(
        {"avr-objcopy", "-O", "ihex", cf_.object_file_.c_str(), cf_.hex_file_.c_str()}, //command + args
        subprocess::error{"generate_hex_errors.txt", false}); //redirect errors if there's a problem generating the hex
    generate_hex.wait(); //wait for generation to finish
}

void file_manager::generate_lst(std::string arch)
{
    auto generate_lst = subprocess::Popen(
        {"avr-objdump", "-m", arch.c_str(), "-S", "-l", "-d", "-g", cf_.object_file_.c_str()}, //specify command and arguments
        subprocess::output{cf_.listing_file_.c_str(), false}, //redirect output 
        subprocess::error{"lst_errors.txt", false}); //redirect errors if there's a problem generating the lst file
    generate_lst.wait(); //wait for generation to finish
}