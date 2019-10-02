#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <set>
#include <map>
#include <stack>
#include <tuple>
#include <fstream>
#include <sstream>

#include "Commun.hpp"

class DetAutomaton {
private:
    std::map<int, std::map<char, int>> transition_table;
    int start_state = -1;
    std::set<int> end_states;
public:
    explicit DetAutomaton() {
        this->clear();
    }

    // load automaton from file
    bool load(std::string file_name) {
        this->clear();
        std::ifstream in(file_name);
        if (!in.is_open()) return false;
        std::string line;
        std::getline(in, line);
        if (line != "Derteministic automaton") {
            std::cout << "error : file " << file_name << " doesn't contain a deterministic automaton" << std::endl;
            return false;
        }

        // read start state
        std::getline(in, line);
        this->start_state = std::stoi(line);

        // read final states
        int nb_end_states;
        std::getline(in, line);
        nb_end_states = std::stoi(line);
        for (int i = 0; i < nb_end_states; ++i) {
            std::getline(in, line);
            this->end_states.insert(std::stoi(line));
        }
        
        // read transitions
        while (std::getline(in, line)) {
            std::istringstream iss(line);
            int s1, s2;
            char c;
            iss >> s1 >> c >> s2;
            this->add_transition(s1, c, s2);
        }
        return true;
    }

    bool save(std::string file_name) {
        std::ofstream out(file_name);
        if (!out.is_open()) return false;
        out << "Derteministic automaton\n";
        out << start_state << "\n";
        out << end_states.size() << "\n";
        for (int i : end_states) out << i << "\n";
        for (auto& [s1, c_s2] : this->transition_table) {
            for (auto& [c, s2] : c_s2) {
                out << s1 << " " << c << " " << s2 << "\n";
            }
        }
        out.close();
        return true;
    }

    int match(std::string str, int offset = 0) {
        if (start_state == -1) return -1;
        int curr = start_state;
        int str_pos = offset;
        int last_matched = -1;
        while (str_pos < str.size()) {
            int next_state = -1;
            if (end_states.find(curr) != end_states.end()) last_matched = str_pos;
            if (this->transition_table[curr].find(str[str_pos]) != this->transition_table[curr].end()) next_state = this->transition_table[curr][str[str_pos]];
            else if (this->transition_table[curr].find(MATCH_OTHERS) != this->transition_table[curr].end()) next_state = this->transition_table[curr][MATCH_OTHERS];
            else break;
            str_pos++;
            curr = next_state;
        }
        if (end_states.find(curr) == end_states.end()) return last_matched != -1 ? last_matched - offset : -1;
        return str_pos - offset;
    }

    void clear() {
        transition_table.clear();
        end_states.clear();
        start_state = -1;
    }

    void set_start_state(int s) {
        this->start_state = s;
    }

    int get_start_state() {
        return this->start_state;
    }

    void add_end_state(int s) {
        this->end_states.insert(s);
    }

    void set_end_states(std::set<int> states) {
        this->end_states = states;
    }

    std::set<int> get_end_states() {
        return this->end_states;
    }
    
    void add_transition(int s1, char c, int s2) {
        transition_table[s1][c] = s2;
        // make sure the second state is added to the automaton
        transition_table[s2];
    }

    std::map<int, std::map<char, int>> get_transition_table() {
        return transition_table;
    }

    int get_next_state(int s, char c) {
        if (transition_table[s].find(c) == transition_table[s].end()) return {};
        return transition_table[s][c];
    }

    int get_states_count() {
        return (int)transition_table.size();
    }

    std::set<int> get_states() {
        std::set<int> res;
        for (auto& [k, v] : transition_table) res.insert(k);
        return res;
    }

    void print() {
        std::cout << "deterministic automaton " << std::endl;
        std::cout << "start state : " << start_state << std::endl;
        std::cout << "end states : ";
        for (int i : end_states) std::cout << i << " ";
        std::cout << std::endl;
        std::cout << "transition table : " << std::endl;
        for (auto& [s1, m] : this->transition_table) {
            std::cout << s1 << " => ";
            for (auto [c, s2] : m) {
                std::cout << "(" << c << ", " << s2 << ")" " ";
            }
            std::cout << std::endl;
        }
    }
};
