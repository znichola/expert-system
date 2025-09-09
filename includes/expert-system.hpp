#ifndef EXPERT_SYSTEM_HPP
# define EXPERT_SYSTEM_HPP

# include "expression.hpp"

// tokenizer returns a vector of Tokens
struct Token {
    std::vector<std::pair<std::string, size_t>> token_list;
    // whatever the token needs
};


// parseRules returns a vector of Rules
struct Rule {
    // A rule is one line in the input file
    // whatever a rule needs, AST etc..
};


// parseFacts returns a vector of Facts
struct Fact {
    // A fact is a statment about the value
    // of a variable from the input file
};


// parseQueries returns a vector of Query
struct Query {
    // A query is the result for a variable
};

Token tokenizer(std::string input);

#endif /* EXPERT_SYSTEM_HPP */
