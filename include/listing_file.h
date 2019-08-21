#ifndef LISTING_FILE_H
#define LISTING_FILE_H

#include <string>
#include <vector>
#include <memory>
class listing_file 
{
    public:

        enum class instruction_type 
        {
            INSTRUCTION,
            LABEL
        };


        struct instruction {
            int line_;
            int byte_address_;
            std::string opcode_;
            std::string instr_;
            instruction_type type_;
            instruction()
                : line_(0), byte_address_(0), opcode_(""), instr_(""), type_(instruction_type::INSTRUCTION) {}

            instruction(int line, int byte_address, std::string opcode, std::string instr)
                : line_(line), byte_address_(byte_address), opcode_(opcode), instr_(instr), type_(instruction_type::INSTRUCTION) {}
        };

        struct label : instruction {
            std::string label_;

            label(int byte_address, std::string label) : label_(label)
            {
                byte_address_ = byte_address;
                type_ = instruction_type::LABEL;
            }
        };

        listing_file(){}
        int size() {return prog_.size();}
        instruction& operator[] (const int index);
        void set_source_file(const std::string& file_name) {file_name_ = file_name;}
        std::vector<instruction>::iterator begin() {return prog_.begin();}
        std::vector<instruction>::iterator parse_lst();
    private:
        std::string file_name_;
        std::vector<instruction> prog_;





};

#endif