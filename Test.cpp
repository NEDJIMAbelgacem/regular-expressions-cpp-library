#include <iostream>
#include <regex>
#include <set>
#include "regex_lib/Regex.hpp"

#define DEFAULT_SEED 0
#define DEFAULT_MIN_NUM 1
#define DEFAULT_MAX_NUM 4
#define DEFAULT_CHARSET_SIZE 3
#define DEFAULT_NB_PIPES 3
#define DEFAULT_NB_CONCATS 3

// TODO : make a probabilistic randomizer (not the standard uniform rand())
// generates a random regex using an alphabet
class RandomRegexGenerator {
private:
    std::vector<char> alphabet;
private:
    std::string gen_num(int min = DEFAULT_MIN_NUM, int max = DEFAULT_MAX_NUM) {
        return std::to_string(min + rand() % (max - min + 1));
    }
    std::string gen_char_set() {
        std::string res;
        for (char c : alphabet) {
            if (rand()) res.push_back(c);
        }
        return res;
    }
    std::string gen_alpha(int nb_parens) {
        int choice = rand() % 4;
        std::string res = "";
        switch (choice)
        {
        case 0:
            // generate parens
            if (nb_parens > 0) {
                res.push_back('(');
                res += gen_expr(nb_parens - 1, DEFAULT_NB_PIPES, DEFAULT_NB_CONCATS);
                res.push_back(')');
                break;
            }
        case 1:
            res.push_back(alphabet[rand() % alphabet.size()]);
            break;
        case 2:
            res += "[";
            res += gen_char_set();
            res += "]";
            break;
        case 3:
            res += "[^";
            res += gen_char_set();
            res += "]";
            break;
        }
        return res;
    }
    std::string gen_unary_expr() {
        int choice = rand() % 4;
        switch (choice)
        {
        case 0: return "+";
        case 1: return "?";
        case 2: return "*";
        case 3: return std::string() + "{" + gen_num() + "}";
        case 4: return std::string() + "{" + gen_num() + ", " + gen_num() + "}";
        default:
            break;
        }
    }
    std::string gen_expr_wo_concat(int nb_parens) {
        std::string alpha = gen_alpha(nb_parens);
        std::string opr = gen_unary_expr();
        return alpha + opr;
    }
    std::string gen_expr_wo_pipes(int nb_parens, int nb_concats) {
        std::string res;
        for (int i = 0; i < nb_concats; ++i) {
            res += gen_expr_wo_concat(nb_parens);
        }
        return res;
    }
    std::string gen_expr(int nb_parens, int nb_pipes, int nb_concats) {
        std::string res;
        for (int i = 0; i < nb_pipes; ++i) {
            res += gen_expr_wo_pipes(nb_parens, nb_concats);
            res += "|";
        }
        if (res.back() == '|') res.pop_back();
        return res;
    }
public:
    RandomRegexGenerator(std::set<char> alphabet) {
        this->alphabet = std::vector<char>(alphabet.begin(), alphabet.end());
    }
    std::string generate_regexp(int nb_parens, int nb_pipes, int nb_concats, int seed = DEFAULT_SEED) {
        srand(seed);
        return gen_expr(nb_parens, nb_pipes, nb_concats);
    }
};

int main()
{
	std::set<char> alphabet = {'a', 'b', 'c'};
    RandomRegexGenerator gen(alphabet);
    std::string regexp = gen.generate_regexp(1, 3, 3, 1);
    std::cout << regexp << std::endl;
    Regex reg(regexp);
    reg.print_automaton();
    std::cin.get();
	return 0;
}