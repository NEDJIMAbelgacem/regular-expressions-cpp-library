#pragma once
#include "Node.hpp"

#pragma once

#include "Commun.hpp"

class NDetAutomaton {
private:
    std::map<int, std::map<char, std::set<int>>> transition_table;
    int start_state = -1;
    std::set<int> end_states;
public:
    explicit NDetAutomaton() {
        transition_table.clear();
    }

    void set_start_state(int s) {
        this->start_state = s;
    }

    void add_end_state(int s) {
        this->end_states.insert(s);
    }
    
    void add_transition(int s1, char c, int s2) {
        transition_table[s1][c].insert(s2);
        // make sure the state is considered
        transition_table[s2];
    }

    std::map<int, std::map<char, std::set<int>>> get_transition_table() {
        return transition_table;
    }

    std::set<int> get_next_state(int s, char c) {
        if (transition_table[s].find(c) == transition_table[s].end()) return {};
        return transition_table[s][c];
    }

    int get_start_state() {
        return this->start_state;
    }

    std::set<int> get_end_states() {
        return this->end_states;
    }

    int get_states_count() {
        return (int)transition_table.size();
    }

    std::set<int> get_states() {
        std::set<int> res;
        for (auto& [k, v] : transition_table) res.insert(k);
        return res;
    }

    std::set<int> closure(std::set<int> states_set) {
        std::set<int> res = states_set;
        std::stack<int> states;
        for (int k : states_set) states.push(k);
        while (!states.empty()) {
            int current = states.top();
            states.pop();
            for (int s : this->transition_table[current][EPSILON]) {
                if (res.find(s) == res.end()) {
                    res.insert(s);
                    states.push(s);
                }
            }
        }
        return res;
    }

    std::set<int> moves(std::set<int> states_set, char alpha) {
        std::set<int> res;
        for (int s1 : states_set) {
            for (int s2 : transition_table[s1][alpha]) {
                res.insert(s2);
            }
        }
        return res;
    }

    std::tuple<int, std::set<int>, std::map<int, std::map<char, std::set<int>>>> copy_automaton(int start, std::set<int> end) {
        std::map<int, std::map<char, std::set<int>>> res;
        std::map<int, int> id_mapping;
        std::set<int> visited;
        std::stack<int> to_be_visited;
        to_be_visited.push(start);
        while (!to_be_visited.empty()) {
            int curr = to_be_visited.top();
            to_be_visited.pop();
            if (visited.find(curr) != visited.end()) continue;
            visited.insert(curr);
            if (id_mapping.find(curr) == id_mapping.end()) id_mapping[curr] = generate_uid();
            for (auto [c, next_states] : transition_table[curr]) {
                for (int s : next_states) {
                    if (id_mapping.find(s) == id_mapping.end()) id_mapping[s] = generate_uid();
                    res[id_mapping[curr]][c].insert(id_mapping[s]);
                    if (visited.find(s) == visited.end()) to_be_visited.push(s);
                }
            }
        }
        std::set<int> end_set;
        for (int e : end) end_set.insert(id_mapping[e]);
        return {id_mapping[start], end_set, res};
    }

    std::pair<int, std::set<int>> copy_automaton_inplace(int start, std::set<int> end) {
        std::map<int, int> id_mapping;
        std::set<int> visited;
        std::stack<int> to_be_visited;
        to_be_visited.push(start);
        while (!to_be_visited.empty()) {
            int curr = to_be_visited.top();
            to_be_visited.pop();
            if (visited.find(curr) != visited.end()) continue;
            visited.insert(curr);
            if (id_mapping.find(curr) == id_mapping.end()) id_mapping[curr] = generate_uid();
            for (auto [c, next_states] : transition_table[curr]) {
                for (int s : next_states) {
                    if (id_mapping.find(s) == id_mapping.end()) id_mapping[s] = generate_uid();
                    this->add_transition(id_mapping[curr],  c, id_mapping[s]);
                    if (visited.find(s) == visited.end()) to_be_visited.push(s);
                }
            }
        }
        std::set<int> end_set;
        for (int e : end) end_set.insert(id_mapping[e]);
        return {id_mapping[start], end_set};
    }

    void print() {
        std::cout << "non deterministic automaton " << std::endl;
        std::cout << "start state : " << start_state << std::endl;
        std::cout << "end states : ";
        for (int i : end_states) std::cout << i << " ";
        std::cout << std::endl;
        std::cout << "transition table : " << std::endl;
        for (auto& [s1, m] : this->transition_table) {
            std::cout << s1 << " => ";
            for (auto [c, states] : m) {
                for (int s2 : states) {
                    std::cout << "(" << c << ", " << s2 << ")" " ";
                }
            }
            std::cout << std::endl;
        }
    }
};