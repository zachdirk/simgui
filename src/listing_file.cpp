#include "listing_file.h"
#include "file_manager.h"
#include <fstream>
#include <iostream>

std::vector<listing_file::instruction>::iterator listing_file::parse_lst()
{
    std::ifstream f(file_name.c_str());
    if (!(f.good()))
    {
        throw std::runtime_error("Problem opening listing file to parse.\n");
    }
    std::string line;
    std::string line_number;
    std::string byte_address;
    std::string opcode;
    std::string instr;
    int i = 0;
    int j = 0;
    while (getline(f, line))
    {  
        //if we reached the end of the program, break
        if (line == "DEFINED SYMBOLS") break;
        line_number = "";
        byte_address = "";
        opcode = "";
        instr = "";
        //start at the beginning of the line
        i = 0;
        //skip the whitespace
        while (line[i] == ' ' || line[i] == '\t') {++i;}
        //j points at the first digit of the line number
        j = i;
        //traverse the line number
        while (line[i] != ' ') {++i;}
        //substr second parameter is number of characters to copy
        line_number = line.substr(j, i - j); 
        //skip the space
        ++i;
        //grab the byte address
        byte_address = line.substr(i, 4);
        //skip past the byte address and space
        i+=5;
        //grab the guaranteed 2 bytes
        j = i; //need this in case there are more bytes
        opcode = line.substr(i, 4);
        
        //skip past the first 2 bytes of the opcode, and space
        i+=5;
        
        //test if there are 2 more bytes
        if (line[i] != ' ' && line[i] != '\t')
        {
            opcode = line.substr(j, 9);
            i+=4;
        }
        //skip the whitespace
        while (line[i] == ' ' || line[i] == '\t') {++i;}
        //the instruction will be the remainder of the line
        instr = line.substr(i);
        //construct the object
        instruction parsed_instruction(std::stoi(line_number), std::stoi(byte_address, nullptr, 16), opcode, instr);
        //put it in the vector
        prog.push_back(parsed_instruction);
    }
    return prog.begin();
}