#include "listing_file.h"
#include "file_manager.h"
#include <fstream>
#include <iostream>
#include <regex>
listing_file::instruction& listing_file::operator[] (const int index)
{
    return prog_[index];
}

std::vector<listing_file::instruction>::iterator listing_file::parse_lst()
{
    prog_.clear();
    std::ifstream f(file_name_.c_str());
    //todo handle gracefully
    if (!(f.good()))
    {
        throw std::runtime_error("Problem opening listing file to parse.\n");
    }
    std::string line;
    std::string line_number;
    std::string byte_address;
    std::string opcode;
    std::string instr;
    std::string str;
    f.seekg(0, std::ios::end);
    str.reserve(f.tellg());
    f.seekg(0, std::ios::beg);
    str.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    std::regex instruction_regex("[a-zA-Z][a-zA-Z0-9]*.S:([0-9]+)\n([\\s\\S]*?) ([0-9a-f]+):\t(([a-f0-9]{2} ){2,})"); //yeah it's ugly
    auto instructions_begin = std::sregex_iterator(str.begin(), str.end(), instruction_regex);
    auto instructions_end = std::sregex_iterator();
    for (std::sregex_iterator i = instructions_begin; i != instructions_end; ++i) 
    {
        std::string line_number = i->str(1);
        char* p;
        int ln = std::strtol(line_number.c_str(), &p, 10); 
        std::string line = i->str(2);
        std::string mem = i->str(3);
        int address = std::strtol(mem.c_str(), &p, 16);
        std::string opcode = i->str(4);
        //pretty sure this is for stripping whitespace but honestly I can't remember
        opcode.erase(std::remove_if(opcode.begin(), opcode.end(), ::isspace), opcode.end());
        listing_file::instruction new_instruction(ln, address, opcode, line);
        prog_.push_back(new_instruction);
    }
    return prog_.begin();
}