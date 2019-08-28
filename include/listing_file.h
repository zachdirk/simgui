#ifndef LISTING_FILE_H
#define LISTING_FILE_H

#include <string>
#include <vector>
#include <memory>

//a class for creating a listing file
//this could probably be a function and a bunch of static functions
class listing_file 
{
    public:

        //instruction here means what I want to display to users in simgui
        struct instruction {
            int line_; //original line number in the .S file
            int byte_address_; //even though flash memory is word addressable it's given in the byte address
            std::string opcode_; //opcode in hex format
            std::string instr_; //instruction text, includes all lines preceding it as well
            instruction() //default constructor
                : line_(0), byte_address_(0), opcode_(""), instr_("") {}

            instruction(int line, int byte_address, std::string opcode, std::string instr)
                : line_(line), byte_address_(byte_address), opcode_(opcode), instr_(instr) {}
        };

        listing_file(){} //default constructor does nothing
        //getters
        int size() {return prog_.size();}
        instruction& operator[] (const int index);
        std::vector<instruction>::iterator begin() {return prog_.begin();}
        
        //setters
        void set_source_file(const std::string& file_name) {file_name_ = file_name;}

        //this function does all the work by parsing the file with regex and building the list of instructions
        std::vector<instruction>::iterator parse_lst(); 
    private:
        std::string file_name_; //the name of hte listing file
        std::vector<instruction> prog_; //the vector of instructions to be returned to simgui





};

#endif