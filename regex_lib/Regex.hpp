#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <set>
#include <map>
#include <stack>
#include <tuple>
#include <regex>

#include "Node.hpp"
#include "NDetAutomaton.hpp"
#include "DetAutomaton.hpp"
#include "RegexParser.hpp"

class Regex {
    RegexParser* parser = nullptr;
    DetAutomaton automaton;
private:
    Regex() {}
public:
    static Regex* load(std::string file_path);
    Regex(std::string regexp) {
        parser = new RegexParser(regexp);
        parser->parse();
        parser->convert_to_nda();
        parser->nd_automaton.print();
        parser->convert_to_determistic(automaton);
    }

    void print_nda() {
        parser->nd_automaton.print();
    }

    void print_automaton() {
        automaton.print();
    }

    int match(std::string str, int offset = 0) {
        return automaton.match(str, offset);
    }

    void save(std::string file_path) {
        automaton.save(file_path);
    }
};

// Warning : returned pointer needs to be deallocated after usage
Regex* Regex::load(std::string file_path) {
    Regex* reg = new Regex;
    reg->automaton.load(file_path);
    return reg;
}