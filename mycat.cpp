#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string symbol_data;
        std::ifstream in(argv[i]);
        std::getline(in, symbol_data, std::string::traits_type::to_char_type(
                                      std::string::traits_type::eof()));
        in.close();
        std::cout << symbol_data;
    }
    return 0;
}
