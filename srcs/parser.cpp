#include <vector>
#include <string>


#include "expert-system.hpp"


std::vector<Rule> parseRules(const Token &input) {
    (void)input;
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
std::vector<Fact> parseFacts(const Token &input) {
    vector<Fact> fact;
    bool found = false;
    size_t line_number = 0;

    for (const auto &token : input.token_list) {
        if (token.first == "=") {
            if (found)
                throw std::runtime_error("Multiple facts definitions found, line: " + std::to_string(token.second));
            line_number = token.second;
            found = true;
            continue;
        }
        if (line_number == token.second && found) {
            char c = token.first[0];
            if (c >= 'A' && c <= 'Z') {
                fact.push_back(Fact(c, Fact::State::True, token.second, ""));
            }
            else if (c == '#')
            {
                for (auto &f : fact)
                    f.comment = token.first.substr(1); // skip the '#'
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
std::vector<Query> parseQueries(const Token &input) {
    vector<Query> queries;
    bool found = false;
    size_t line_number = 0;

    for (const auto &token : input.token_list) {
        if (token.first == "?") {
            if (found)
                throw std::runtime_error("Multiple queries definitions found, line: " + std::to_string(token.second));
            line_number = token.second;
            found = true;
            continue;
        }
        if (line_number == token.second && found) {
            char c = token.first[0];
            if (c >= 'A' && c <= 'Z') {
                queries.push_back(Query(c, token.second, ""));
            }
            else if (c == '#')
            {
                for (auto &f : queries)
                    f.comment = token.first.substr(1); // skip the '#'
            }
                
            else {
                throw std::runtime_error("Invalid character in queries: " + std::string(1, c));
            }
            
        }
    }
    return queries;
}

