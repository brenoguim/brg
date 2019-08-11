#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    std::string symbol_data;
    std::ifstream in(argv[1]);
    std::getline(in, symbol_data, std::string::traits_type::to_char_type(
                                  std::string::traits_type::eof()));
    in.close();
    std::cout << symbol_data << std::endl;
    return 0;
}
