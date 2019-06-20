#include <string>


class file_manager 
{

public:

    enum class file_type 
    {
        C,
        ASM,
        S,
        OTHER
    };

    struct compile_args 
    {
        std::string mmcu;
        file_type type;
        bool omit_false_conditionals;//c
        bool omit_debugging_directives;//d
        bool include_general_information;//g
        bool include_highlevel_source;//h
        bool include_assembly;//l
        bool include_macro_expansions;//m
        bool omit_forms_processing;//n
        bool include_symbols;//s
        bool keep_locals; //-L
        bool debug;//-g
        bool save_temps;//-save-temps
        bool verbose_asm;//-fverbose-asm
        std::string file_name;//=file

        //default parameters to make it easy for me
        compile_args() :    mmcu("atmega2560"), 
                            type(file_type::C), 
                            omit_false_conditionals(false),
                            omit_debugging_directives(true),
                            include_general_information(false), 
                            include_highlevel_source(true),
                            include_assembly(true),
                            include_macro_expansions(false),
                            omit_forms_processing(true),
                            include_symbols(true),
                            keep_locals(true),
                            debug(true),
                            save_temps(false),
                            verbose_asm(true),
                            file_name("")  {}
    };

    struct compilation_files {
        file_type type;
        std::string no_extension;
        std::string source_file; 
        std::string object_file;
        std::string assembly_file;
        std::string listing_file;
        std::string hex_file;
    };

private:

    bool good;
    compilation_files cf;     


public:
    //static functions
    static file_type get_file_type(const std::string& file_name);
    static std::string get_file_name_no_extension(const std::string& file_name);
    static bool file_exists(const std::string& file_name);
    //constructors
    file_manager(std::string& file_name);    
    
    //getters
    bool is_good() {return good;}
    file_type compilation_file_type() {return cf.type;}

    //compilation functions
    bool compile(const compile_args& c);
    bool assemble();
    bool generate_hex();
    
};

