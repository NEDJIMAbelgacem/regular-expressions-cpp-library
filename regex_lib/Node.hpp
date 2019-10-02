#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <set>
#include <map>
#include <stack>
#include <tuple>

enum NodeType {Pipe, Concat, StarRep, OptRep, PlusRep, ValRep, BoundedRep, CharSelect, CharExcl, Char};

class Node;
using NodeValVariant = std::variant<std::set<char>, char, int, std::pair<int, int>, std::vector<Node*>>;

class Node {
public:
    Node(NodeType t_type, NodeValVariant t_value) : type(t_type), val(t_value) { }

    Node(NodeType t_type) : type(t_type) { }

    NodeType type;
    NodeValVariant val;
    std::vector<Node*> operands;
};