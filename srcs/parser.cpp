#include <vector>
#include <string>
# include "parser.hpp"


#include "expert-system.hpp"

std::tuple<size_t, vector<Token>, string> getNextLine(const vector<Token> &tokens, size_t index);


/*
** parseTokens implementation
** ----------------------------
** This function takes a vector of Token structs and returns a tuple containing:
** - A vector of Rule structs
** - A vector of Fact structs
** - A vector of Query structs
** It processes the tokens to identify rules, facts, and queries based on their types and line numbers.
** It throws exceptions for syntax errors, such as missing facts or queries.
*/
std::tuple<vector<Rule>, vector<Fact>, vector<Query>>
    parseTokens(const vector<Token> &input) {
    // take the list of tokens, split into lines then keep the comments to one side, and 
    // using the Parser.parse()
    if (input.empty())
        return {};
    vector<Rule> rules;
    vector<Query> queries = parseQueries(input);
    vector<Fact> facts = parseFacts(input);

    size_t facts_line_number = -1;
    if (queries.empty())
        throw std::runtime_error("No queries found in input");
    for (auto const &i : input) {
        if (i.type == Token::Type::Fact) {
            facts_line_number = i.line_number;
        }
    }

    
    size_t i = 0;
    while (i < input.size())
    {
        if (input[i].line_number == queries[0].line_number ||
            input[i].line_number == facts_line_number) {
            i++;
            continue; // skip facts and queries lines
        }
        auto [newI, lineTokens, comment] = getNextLine(input, i);
        i = newI;
        if (!lineTokens.empty()) {
            // TODO remove this now that the parser can do precedence
            // add parentheses around the entire expression
            // to be sure that the conclusion and the premis are well separated
            lineTokens.push_back(Token(")", lineTokens[0].line_number, Token::Type::Parenthese));
            lineTokens.insert(lineTokens.begin(), Token("(", lineTokens[0].line_number, Token::Type::Parenthese));
            // insert parentheses before and after "=>", "<=>"
            for (size_t j = 0; j < lineTokens.size(); j++) {
                if (lineTokens[j].token_list == "=>" || lineTokens[j].token_list == "<=>") {
                    lineTokens.insert(lineTokens.begin() + j, Token(")", lineTokens[0].line_number, Token::Type::Parenthese));
                    lineTokens.insert(lineTokens.begin() + j + 2, Token("(", lineTokens[0].line_number, Token::Type::Parenthese));
                    j += 2; // skip the newly added tokens
                }
            }
            Parser parser{0, lineTokens};

            try {
                Expr expr = parser.parse();
                rules.push_back(Rule(expr, lineTokens[0].line_number, comment));
                // printf("Parsed rule: %s\n", rules.back().toString().c_str());
            } catch (const std::exception &e) {
                std::stringstream ss;
                ss << "Line: " << lineTokens[0].line_number << " :" << e.what() << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
    }

    return {rules, facts, queries} ;
};


// make a get next line function that takes a tokens vector and return a new vector of just the line and the new index
std::tuple<size_t, vector<Token>, string> getNextLine(const vector<Token> &tokens, size_t index) {
    vector<Token> line_tokens;
    size_t line_number = tokens[index].line_number;
    string comment = "";
    while (index < tokens.size() && tokens[index].line_number == line_number) {
        if (tokens[index].type != Token::Type::NewLine)
            line_tokens.push_back(tokens[index]);
        index++;
    }
    if (!line_tokens.empty() && line_tokens.back().type == Token::Type::Comment) {
        comment = line_tokens.back().token_list.substr(1); // skip the '#'
        line_tokens.pop_back();
    }
    return {index, line_tokens, comment};
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

