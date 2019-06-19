#include <string>


class file_manager 
{

public:

enum class file_type 
{
    C,
    ASM,
    S
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
    bool keep_locals;
    std::string file_name;//=file
};

private:

file_type file_type;
std::string file_name;
std::string file_extension;

public:
    
    file_manager(std::string& file_name);

    bool compile(compile_args c);

    
};

