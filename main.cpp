#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <set>
#include <map>
#include <stack>
#include <tuple>

#include "regex_lib/Regex.hpp"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        std::cout << "Please specify the regular expression and a string as CLI argument" << std::endl;
        std::cout << argc << std::endl;
        exit(0);
    }

    std::string regexp(argv[1]);
    std::string input(argv[2]);
    std::cout << "the regular expression : " << regexp << std::endl;
    std::cout << "the input string : " << input << std::endl;
    Regex reg(regexp);
    std::cout << "matched string length : " << reg.match(input) << std::endl; 
    std::cin.get();
    return 0;
}
