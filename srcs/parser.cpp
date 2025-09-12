#include <vector>
#include <string>
# include "parser.hpp"


#include "expert-system.hpp"

std::tuple<vector<Rule>, vector<Fact>, vector<Query>>
    parseTokens(const vector<Token> &input) {
    // take the list of tokens, split into lines then keep the comments to one side, and 
    // using the Parser.parse()
    if (input.empty())
        return {};
    vector<Rule> rules;
    vector<Query> queries = parseQueries(input);
    vector<Fact> facts = parseFacts(input);
    if (queries.empty())
        throw std::runtime_error("No queries found in input");
    if (facts.empty())
        throw std::runtime_error("No facts found in input");

    for (size_t i = 0; i < input.size(); i++)
    {   size_t line = input[i].line_number;
        string comment;
        if (line == queries[0].line_number || line == facts[0].line_number)
            continue;
        vector<Token> line_tokens;
        while (i < input.size() && input[i].type != Token::Type::NewLine )
        {
            line_tokens.push_back(input[i]);
            i++;
        }
        // if previous token was comment, remove it from line_tokens and save it
        if (line_tokens.back().type == Token::Type::Comment)
        {
            comment = line_tokens.back().token_list.substr(1); // skip the '#'
            line_tokens.pop_back();
        }
        Parser parser{0, line_tokens};
        Expr expr = parser.parse();
        rules.push_back(Rule(expr, line, comment));
    }

    return {rules, facts, queries} ;
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
std::vector<Fact> parseFacts(const vector<Token> &input) {
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
std::vector<Query> parseQueries(const vector<Token> &input) {
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

