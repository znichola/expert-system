#include <vector>
#include <string>


#include "expert-system.hpp"

// Rule getNextVar(const vector<Token> &tokens, size_t &index, int line_number)
// {

// }

std::vector<Rule> parseRules(const vector<Token> input) {
    (void)input;
    /*int query_line = -1;
    int fact_line = -1;
    std::vector<Rule> rules;
    Rule current_rule(Expr{Var{' '}}); // dummy initialization
    Rule next_rule(Expr{Var{' '}}); // dummy initialization

    char prev_var = '\0';
    bool prev_was_var = false;

    for (size_t i = 0; i < input.size(); i++) {
        const Token &token = input[i];
        if (token.line_number == query_line || token.line_number == fact_line)
            continue;
        const char c = token.token_list[0];
        switch(token.type) {
            case Token::Type::Variable:
                prev_var = c;
                if (prev_was_var)
                    throw std::runtime_error("Two consecutive variables found, line: " + std::to_string(token.line_number));
                prev_was_var = true;
                current_rule.expr = Expr{Var{c}};
                current_rule.line_number = token.line_number;
                break;
            
            case Token::Type::Operator:
                prev_was_var = false;
                
                break;
            
            case Token::Type::Fact:
                if (fact_line != -1)
                    throw std::runtime_error("Multiple fact definitions found, line: " + std::to_string(token.line_number));
                fact_line = token.line_number;
                prev_was_var = false;
                break;
            
            case Token::Type::Query:
                prev_was_var = false;
                if (query_line != -1)
                    throw std::runtime_error("Multiple query definitions found, line: " + std::to_string(token.line_number));
                query_line = token.line_number;
                break;
            
            case Token::Type::Parenthese:
                prev_was_var = false;
                break;
            
            case Token::Type::Comment:
                prev_was_var = false;
                current_rule.comment = token.token_list.substr(1); // skip the '#'
                break;
            
            case Token::Type::NewLine:
                prev_was_var = false;
                if (current_rule.expr != Expr{Var{' '}}) { // if current_rule is not the dummy
                    rules.push_back(current_rule);
                    current_rule = Rule(Expr{Var{' '}}); // reset to dummy
                }
                break;
            default:
                throw std::runtime_error("Unknown token type");
        }
    }
    if (current_rule.expr != Expr{Var{' '}}) { // if current_rule is not the dummy
        rules.push_back(current_rule);
    }
    return rules;
    */
   return {};
}

/*
** parseFacts implementation
** ----------------------------
** This function takes as input a Token struct and returns a vector of Fact structs.
** It looks for the '=' token to identify the start of the facts section.
** It then collects all uppercase letters (A-Z) that appear on the same line as the '=' token.
** It also captures comments that start with '#' on the same line.
** It throws an exception if multiple '=' tokens are found or if invalid characters are encountered.
*/
std::vector<Fact> parseFacts(const vector<Token> input) {
    vector<Fact> fact;
    bool found = false;
    size_t line_number = 0;

    for (const auto &token : input) {
        if (token.token_list == "=") {
            if (found)
                throw std::runtime_error("Multiple facts definitions found, line: " + std::to_string(token.line_number));
            line_number = token.line_number;
            found = true;
            continue;
        }
        if (line_number == token.line_number && found) {
            char c = token.token_list[0];
            if (c >= 'A' && c <= 'Z') {
                fact.push_back(Fact(c, Fact::State::True, token.line_number, ""));
            }
            else if (c == '#')
            {
                for (auto &f : fact)
                    f.comment = token.token_list.substr(1); // skip the '#'
            }
                
            else {
                throw std::runtime_error("Invalid character in facts: " + std::string(1, c));
            }
            
        }
    }
    return fact;
}


/*
** parseQueries implementation
** ----------------------------
** This function takes as input a Token struct and returns a vector of Query structs.
** It looks for the '?' token to identify the start of the queries section.
** It then collects all uppercase letters (A-Z) that appear on the same line as the '?' token.
** It also captures comments that start with '#' on the same line.
** It throws an exception if multiple '?' tokens are found or if invalid characters are encountered.
*/
std::vector<Query> parseQueries(const vector<Token> input) {
    vector<Query> queries;
    bool found = false;
    size_t line_number = 0;

    for (const auto &token : input) {
        if (token.token_list == "?") {
            if (found)
                throw std::runtime_error("Multiple queries definitions found, line: " + std::to_string(token.line_number));
            line_number = token.line_number;
            found = true;
            continue;
        }
        if (line_number == token.line_number && found) {
            char c = token.token_list[0];
            if (c >= 'A' && c <= 'Z') {
                queries.push_back(Query(c, token.line_number, ""));
            }
            else if (c == '#')
            {
                for (auto &f : queries)
                    f.comment = token.token_list.substr(1); // skip the '#'
            }
                
            else {
                throw std::runtime_error("Invalid character in queries: " + std::string(1, c));
            }
            
        }
    }
    return queries;
}

