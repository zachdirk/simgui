#include <string>
#include <iostream>

class file_manager 
{

public:

    struct compilation_files {
        std::string no_extension_;
        std::string source_file_; 
        std::string object_file_;
        std::string listing_file_;
        std::string hex_file_;
    };

private:

    compilation_files cf_;     
    //compilation functions


public:
    //
    static std::string get_file_name_no_extension(const std::string& file_name);
    //constructors
    file_manager(){}  
    
    //getters
    std::string get_source_file() {return cf_.source_file_;}
    std::string get_object_file() {return cf_.object_file_;}
    std::string get_lst_file() {return cf_.listing_file_;}
    std::string get_hex_file() {return cf_.hex_file_;}
    //setters
    void open(std::string file_name);
    
    void assemble();
    void generate_hex();
    void generate_lst();
};

