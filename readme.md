## A regular expressions library implemented from scratch in C++

 This regular expressions library is better implementation of the regular expressions engine I made a year ago and have more features. I will be making an article about how I made this library. Also it needs more testing.
 This repository is built to help me in making a compiler from scratch while I'm learning compilation techniques in this semestre's class.

## Features :

Supported operators :
- concatentation operator like "ab".
- union operator like "a|b".
- 0 or more repetitions operator '*'.
- 1 or more repetitions operator '+'.
- optional operator '?'.
- n times repetitions "{n}".
- n1 to n2 times repetitions {n1, n2}.
- '.' matches any character.
- character set like [abc] which matches a or b or c
- exclusion character set like [^abc] which matches any string other than abc
- some character classes like \d which matches digits, \w which is equivalent to [a-zA-Z0-9_] and \a which matches alphabet characters ([a-zA-Z]).

### language grammar
I implemeted a top down parser to convert regular expressions to an abstract syntax tree. The abstract syntax tree is then used to make a non deterministic automaton which is then converted to a deterministic one.

The grammar I used for parsing is :
- start symbol : expr
- non terminals = {expr, choice, opr_expr, char_set, num, char_seq}
- terminals = {|, +, *, ?, ^, {, }, [, ], (, ), digit, char}
- productions set :
{
    expr -> expr_wo_pipe|expr,
    expr_wo_pipe -> expr_wo_concat expr_wo_pipe,
    expr_wo_concat -> alpha | alpha opr_expr,
    opr_expr -> unary_opr | unary_opr opr_expr,
    unary_opr -> + | ? | * | {num} | {num,num},
    alpha -> p_expr | char | [char_set] | [^char_set],
    p_expr -> (expr),
    char_set -> char | char char_set,
    num -> digit | digit num
}

There are some implementation details that are not described in the grammar like '.' character, character classes and escaped characters like \\( which are handled by the lexical parser.
