#ifndef LISTING_FILE_H
#define LISTING_FILE_H

#include <string>
#include <vector>
class listing_file 
{
    public:

        struct instruction {
            int line;
            int byte_address;
            std::string opcode;
            std::string instr;

            instruction(int line, int byte_address, std::string opcode, std::string instr)
                : line(line), byte_address(byte_address), opcode(opcode), instr(instr) {}
        };

        listing_file(std::string file_name) : file_name(file_name) {}

        std::vector<instruction>::iterator parse_lst();
    private:
        std::string file_name;
        std::vector<instruction> prog;





};

#endif