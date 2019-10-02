#include <iostream>
#include <regex>
#include <set>

#include "regex_lib/Regex.hpp"
#include "regex_lib/DetAutomaton.hpp"

int main(int argc, const char** argv) {
    if (argc != 3) {
        std::cout << "Please specify the regular expression and a string as CLI argument" << std::endl;
        std::cout << argc << std::endl;
        std::cin.get();
        exit(0);
    }

    std::string choice = argv[1];
    if (choice == "save") {
        std::string regexp("(ab|.?c)+");
        Regex reg(regexp);
        reg.save(argv[2]);
    } else if (choice == "load") {
        Regex* reg = Regex::load(argv[2]);
        reg->print_automaton();
        delete reg;
    }
    std::cin.get();
    return 0;
}