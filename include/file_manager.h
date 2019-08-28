#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <iostream>


//a class for holding all my files (could be a struct tbh)
class file_manager 
{

public:

    struct compilation_files {
        std::string no_extension_; //the file name with no extension
        std::string source_file_; //the source file name
        std::string object_file_; //the object file name
        std::string listing_file_; //the listing file name
        std::string hex_file_; //the hex file name
    };

private:

    //the actual container for the file names
    compilation_files cf_;     


public:
    //a helper function which returns all the characters in a string before the last . 
    //e.g this/is/a/path/to/simgui.S -> this/is/a/path/to/simgui 
    static std::string get_file_name_no_extension(const std::string& file_name);
    //constructors
    file_manager(){} //default constructor initializes files to blank 
    
    //getters
    std::string get_source_file() {return cf_.source_file_;}
    std::string get_object_file() {return cf_.object_file_;}
    std::string get_lst_file() {return cf_.listing_file_;}
    std::string get_hex_file() {return cf_.hex_file_;}
    //setters
    void open(std::string file_name);
    
    void assemble(std::string mmcu); //assembles the file with the given mmcu as a parameter
    void generate_hex(); //generates the hex using avr-objcopy
    void generate_lst(std::string arch); //generates the listing by decompiling the object file
};

#endif