#pragma once

#include "Node.hpp"
#include "NDetAutomaton.hpp"
#include "DetAutomaton.hpp"

#define END_OF_INPUT '\0'
#define DIGIT '\1'
#define ALPHA '\2'
#define ALPHANUM '\3'

class RegexParser {
private:
    std::string regexp;
    int pos;
    char curr;
    Node* ast = nullptr;
public:
    NDetAutomaton nd_automaton;
public:
    RegexParser(std::string expr) : regexp(expr) { }
    ~RegexParser() {}

    void parse() {
        pos = 0;
        curr = current();
        ast = parse_expr();
    }

    void convert_to_nda() {
        auto [start, end] = convert_ast2nda(ast);
        this->nd_automaton.set_start_state(start);
        this->nd_automaton.add_end_state(end);
    }

    void print_syntax_tree() {
        std::vector<Node*> nodes;
        nodes.push_back(ast);
        while (!nodes.empty()) {
            std::vector<Node*> next_nodes;
            for (Node* n : nodes) {
                for (Node* operand : n->operands) next_nodes.push_back(operand);
                switch (n->type)
                {
                case NodeType::Pipe: std::cout << "|" << " "; break;
                case NodeType::Concat: std::cout << "-" << " "; break;
                case NodeType::StarRep: std::cout << "*" << " "; break;
                case NodeType::OptRep: std::cout << "?" << " "; break;
                case NodeType::PlusRep: std::cout << "+" << " "; break;
                case NodeType::ValRep: std::cout << "{" << std::get<int>(n->val) << "}" << " "; break;
                case NodeType::BoundedRep: std::cout << "{" << std::get<std::pair<int, int>>(n->val).first << "," << std::get<std::pair<int, int>>(n->val).second << "} "; break;
                case NodeType::CharSelect: 
                    std::cout << "[";
                    for (char c : std::get<std::set<char>>(n->val)) std::cout << c;
                    std::cout << "] "; 
                    break;
                case NodeType::CharExcl:
                    std::cout << "[^";
                    for (char c : std::get<std::set<char>>(n->val)) std::cout << c;
                    std::cout << "] "; 
                    break;
                case NodeType::Char: std::cout << std::get<char>(n->val) << " "; break;

                default:
                    break;
                }
            }
            std::cout << std::endl;
            nodes = next_nodes;
        }
    }

    void convert_to_determistic(DetAutomaton& d_automaton) {
        std::set<std::set<int>> d_states;
        std::set<int> d_start_state = nd_automaton.closure({nd_automaton.get_start_state()});

        d_states.insert(d_start_state);
        std::stack<std::set<int>> to_be_marked;
        to_be_marked.push(d_start_state);
        std::set<std::set<int>> marked;

        std::map<std::set<int>, int> states_id_mapping;
        states_id_mapping[d_start_state] = generate_uid();

        auto transition_table = nd_automaton.get_transition_table();

        d_automaton.clear();
        while (!to_be_marked.empty()) {
            std::set<int> t = to_be_marked.top();
            to_be_marked.pop();
            if (marked.find(t) != marked.end()) continue;
            marked.insert(t);
            std::map<char, std::set<int>> transitions;
            for (int s1 : t) {
                for (auto [c, states] : transition_table[s1]) {
                    for (int s2 : states) {
                        transitions[c].insert(s2);
                    }
                }
            }

            for (auto& [c, states] : transitions) {
                states = nd_automaton.closure(states);
            }

            int t_id = states_id_mapping[t];
            for (auto [c, d_state] : transitions) {
                if (c == EPSILON) continue;
                if (states_id_mapping.find(d_state) == states_id_mapping.end()) states_id_mapping[d_state] = generate_uid();
                d_automaton.add_transition(t_id, c, states_id_mapping[d_state]);
                if (marked.find(d_state) != marked.end()) continue;
                to_be_marked.push(d_state);
                d_states.insert(d_state);
            }
        }
        d_automaton.set_start_state(states_id_mapping[d_start_state]);
        for (int fs : nd_automaton.get_end_states()) {
            for (auto [state, id] : states_id_mapping) {
                if (state.find(fs) != state.end()) d_automaton.add_end_state(id);
            }
        }
    }

private:
    void fatal_error(std::string err) {
        std::cout << "Regex parser error : " << err << std::endl;
        exit(-1);
    }

    char current() {
        if (pos >= regexp.size()) return END_OF_INPUT;
        char c1 = regexp[pos];
        if (c1 != '\\') return c1;
        if (pos + 1 >= regexp.size()) return END_OF_INPUT;
        char c2 = regexp[pos + 1];
        if (c2 == 'd') return DIGIT;
        if (c2 == 'a') return ALPHANUM;
        if (c2 == 'w') return ALPHA;
        return c2 | 0x80;
    }

    void advance() {
        if (pos >= regexp.size()) return;
        pos++;
        if (regexp[pos - 1] != '\\' || pos >= regexp.size()) return;
        pos++;
    }

    void match(char c) {
        if (curr == END_OF_INPUT) fatal_error("Unexpected end");
        if (curr != c) fatal_error(std::string("expected character ") + std::string(1, c) + " at " + std::to_string(pos));
        advance();
        curr = current();
    }

    bool is_at_expr_wo_pipe_end() {
        if (curr == END_OF_INPUT || curr == '|' || curr == ')') return true;
        return false;
    }
    
    Node* parse_expr() {
        Node* left = this->parse_expr_wo_pipe();
        if (is_at_expr_wo_pipe_end()) {
            if (curr == END_OF_INPUT || curr == ')') return left;
            match('|');
            Node* right = parse_expr();
            Node* pipe = new Node(NodeType::Pipe);
            pipe->operands = {left, right};
            left = pipe;
        } else if (curr != END_OF_INPUT) {
            fatal_error(std::string("error that shouldn\'t happen at ") + __FILE__ + ":" + std::to_string(__LINE__));
        }
        return left;
    }

    bool is_at_expr_wo_concat_end() {
        return curr == END_OF_INPUT || curr == '|' || curr == ')';
    }

    Node* parse_expr_wo_pipe() {
        Node* left = this->parse_expr_wo_concat();
        if (is_at_expr_wo_concat_end()) return left;
        Node* right = this->parse_expr_wo_pipe();
        Node* concat = new Node(NodeType::Concat);
        concat->operands = {left, right};
        return concat;
    }

    bool is_at_opr_expr_start() {
        if (curr == END_OF_INPUT) return false;
        return curr == '+' || curr == '?' || curr == '*' || curr == '{';
    }

    Node* parse_expr_wo_concat() {
        Node* left = this->parse_alpha();
        if (!is_at_opr_expr_start()) return left;
        auto [leftmost, rightmost] = this->parse_opr_expr();
        leftmost->operands = { left };
        return rightmost;
    }

    std::pair<Node*, Node*> parse_opr_expr() {
        Node* left = parse_unary_opr();
        Node* right = left;
        while (is_at_opr_expr_start()) {
            Node* n = parse_unary_opr();
            n->operands = { right };
            right = n;
        }
        return {left, right};
    }
    
    Node* parse_unary_opr() {
        Node* node = nullptr;
        int num1, num2;
        char op_char = curr;
        switch (op_char)
        {
        case '+':
        case '?':
        case '*':
            if (op_char == '+') node = new Node(NodeType::PlusRep, op_char);
            else if (op_char == '*') node = new Node(NodeType::StarRep, op_char);
            else if (op_char == '?') node = new Node(NodeType::OptRep, op_char);
            match(op_char);
            break;
        case '{':
            match('{');
            num1 = parse_num();
            if (!is_at_2nd_num()) {
                node = new Node(NodeType::ValRep, num1);
            } else {
                match(',');
                num2 = parse_num();
                node = new Node(NodeType::BoundedRep, std::make_pair(num1, num2));
            }
            match('}');
            break;
        default:
            break;
        }
        return node;
    }

    Node* parse_alpha() {
        Node* left = nullptr;
        if (curr == END_OF_INPUT) fatal_error("Unexpected end");
        switch (curr)
        {
        case '(':
            match('(');
            left = parse_expr();
            match(')');
            break;
        case '[':
            match('[');
            if (curr == '^') {
                match('^');
                left = new Node(NodeType::CharExcl, parse_char_set());
            } else left = new Node(NodeType::CharSelect, parse_char_set());
            match(']');
            break;
        default:
            if (curr & 0x80) left = new Node(NodeType::Char, (char)(curr & 0x7f));
            else left = new Node(NodeType::Char, curr);
            match(curr);
            break;
        }
        return left;
    }

    bool is_at_2nd_num() {
        if (curr == END_OF_INPUT) return false;
        return curr == ',';
    }

    void skip_white_spaces() {
        while (curr != END_OF_INPUT && curr == ' ') {
            match(curr);
        }
    }

    int parse_num() {
        std::string str;
        skip_white_spaces();
        while (curr != END_OF_INPUT && is_num(curr)) {
            str.push_back(curr);
            match(curr);
        }
        skip_white_spaces();
        return std::stoi(str);
    }

    std::set<char> parse_char_set() {
        std::set<char> res;
        while (curr != END_OF_INPUT && curr != ']') {
            res.insert(curr);
            match(curr);
        }
        return res;
    }

    // return the start & end of the sub tree
    std::pair<int, int> convert_ast2nda(Node* n) {
        switch (n->type)
        {
        case NodeType::Pipe:
            {
                auto [left_start, left_end] = convert_ast2nda(n->operands[0]);
                auto [right_start, right_end] = convert_ast2nda(n->operands[1]);
                int start = generate_uid();
                int end = generate_uid();
                this->nd_automaton.add_transition(start, EPSILON, left_start);
                this->nd_automaton.add_transition(start, EPSILON, right_start);
                this->nd_automaton.add_transition(left_end, EPSILON, end);
                this->nd_automaton.add_transition(right_end, EPSILON, end);
                return {start, end};
            }
            break;
        case NodeType::Concat:
            {
                auto [left_start, left_end] = convert_ast2nda(n->operands[0]);
                auto [right_start, right_end] = convert_ast2nda(n->operands[1]);
                int start = left_start;
                int end = right_end;
                this->nd_automaton.add_transition(left_end, EPSILON, right_start);
                return {start, end};
            }
            break;
        case NodeType::StarRep: 
            {
                auto [start, end] = convert_ast2nda(n->operands[0]);
                this->nd_automaton.add_transition(start, EPSILON, end);
                this->nd_automaton.add_transition(end, EPSILON, start);
                return {start, end};
            }
            break;
        case NodeType::OptRep:
            {
                auto [start, end] = convert_ast2nda(n->operands[0]);
                this->nd_automaton.add_transition(start, EPSILON, end);
                return {start, end};
            }
            break;
        case NodeType::PlusRep:
            {
                auto [start, end] = convert_ast2nda(n->operands[0]);
                this->nd_automaton.add_transition(end, EPSILON, start);
                return {start, end};
            }
            break;
        case NodeType::ValRep:
            {
                int num = std::get<int>(n->val);
                auto [start, end] = convert_ast2nda(n->operands[0]);
                if (num == 0) this->nd_automaton.add_transition(start, EPSILON, end);
                int endi = end;
                for (int i = 1; i < num; ++i) {
                    auto [starti, end_set] = this->nd_automaton.copy_automaton_inplace(start, {end});
                    this->nd_automaton.add_transition(endi, EPSILON, starti);
                    endi = *end_set.begin();
                }
                return {start, endi};
            }
            break;
        case NodeType::BoundedRep:
            {
                {
                    auto [num1, num2] = std::get<std::pair<int, int>>(n->val);
                    std::vector<std::pair<int, int>> chaining;
                    auto [start, end] = convert_ast2nda(n->operands[0]);
                    if (num1 == 0) this->nd_automaton.add_transition(start, EPSILON, end);
                    chaining.push_back({start, end});
                    for (int i = 1; i < num2; ++i) {
                        auto [starti, end_set] = this->nd_automaton.copy_automaton_inplace(start, {end});
                        int endi = *end_set.begin();
                        int last_end = chaining.back().second;
                        this->nd_automaton.add_transition(last_end, EPSILON, starti);
                        chaining.push_back({starti, endi});
                    }
                    end = chaining.back().second;
                    for (int i = num1 - 1; i < chaining.size() - 1; ++i) {
                        this->nd_automaton.add_transition(chaining[i].second, EPSILON, end);
                    }
                    return {start, end};
                }
                break;
            }
            break;
        case NodeType::CharSelect: 
            {
                int start = generate_uid();
                int end = generate_uid();
                for (char c : std::get<std::set<char>>(n->val)) {
                    this->nd_automaton.add_transition(start, c, end);
                }
                return {start, end};
            }
            break;
        case NodeType::CharExcl:
            {
                int start = generate_uid();
                int dead_state = generate_uid();
                int end = generate_uid();
                std::set<char> char_set = std::get<std::set<char>>(n->val);
                for (char c : char_set) {
                    nd_automaton.add_transition(start, c, dead_state);
                }
                nd_automaton.add_transition(start, MATCH_OTHERS, end);
                return {start, end};
            }
        case NodeType::Char:
            {
                int start = generate_uid();
                int end = generate_uid();
                char c = std::get<char>(n->val);
                if (c == DIGIT) {
                    for (char c2 = '0'; c2 <= '9'; ++c2) {
                        this->nd_automaton.add_transition(start, c2, end);
                    }
                } else if (c == ALPHANUM) {
                    for (char c2 = '0'; c2 <= '9'; ++c2) {
                        this->nd_automaton.add_transition(start, c2, end);
                    }
                    for (char c2 = 'a'; c2 <= 'z'; ++c2) {
                        this->nd_automaton.add_transition(start, c2, end);
                    }
                    for (char c2 = 'A'; c2 <= 'Z'; ++c2) {
                        this->nd_automaton.add_transition(start, c2, end);
                    }
                    this->nd_automaton.add_transition(start, '_', end);
                } else if (c == ALPHA) {
                    for (char c2 = 'a'; c2 <= 'z'; ++c2) {
                        this->nd_automaton.add_transition(start, c2, end);
                    }
                    for (char c2 = 'A'; c2 <= 'Z'; ++c2) {
                        this->nd_automaton.add_transition(start, c2, end);
                    }
                } else if (c & 0x80) {
                    this->nd_automaton.add_transition(start, c & 0x7f + 1, end);
                } else {
                    this->nd_automaton.add_transition(start, c, end);
                }
                return {start, end};
            }
            break;
        
        default:
            break;
        }
        return {-1, -1};
    }
};
