#include "expert-system.hpp"
#include <stdexcept>
using std::string;

/*
** Tokenizer implementation
** ----------------------------
** This function takes as input a string containing the entire input file
** and returns a Token struct containing a list of tokens and their line numbers.
** It recognizes the following tokens:
** - Variables: single uppercase letters (A-Z)
** - Operators: <=>, =>, +, |, ^, !
** - Fact and Query: =, ?
** - Parentheses: (, )
** - Comments: starting with # and continuing to the end of the line
** - New lines are tracked for error reporting
** It throws an exception if it encounters an invalid character.
*/

vector<Token> tokenizer(string input)
{
    string  t_authorized[] = {"<=>", "=>","+","|","^","(",")","!","#","=", "?"};
    vector<Token>   tokens;
    bool    find_token = false;
    size_t  line = 0;

    for (size_t  i = 0;input[i]; i++)
    {
        // if we find a new line or an uppercase letter, we add it to the tokens
        if (input[i] == '\n' || (input[i] >= 'A' && input[i] <= 'Z'))
        {
            if (input[i] == '\n')
                line++;
            tokens.push_back(Token(string(1, input[i]), line, input[i] == '\n' ? Token::Type::NewLine : Token::Type::Variable));
        }
        // pass spaces
        else if (isspace(input[i]))
            continue;
        // search for authorized tokens in the input
        else
        {
            for (const string& authorized : t_authorized)
            {
                if (input.substr(i, authorized.length()) == authorized)
                {
                    // if we find a comment, we add everything until the end of the line
                    if (authorized == "#")
                    {
                        tokens.push_back(Token(input.substr(i, input.find('\n', i) - i), line, Token::Type::Comment));
                        i += tokens.back().token_list.length() - 1;
                    }
                    // if we find a multi-character token, we add it to the tokens
                    else
                    {
                        Token::Type type = Token::Type::Operator;
                        if (authorized == "=")
                        {
                            type = Token::Type::Fact;
                            // if previous token is not '\n' error
                            if (!tokens.empty() && tokens.back().type != Token::Type::NewLine)
                                throw std::invalid_argument("Invalid '=' position at line " + std::to_string(line + 1));
                        }
                        else if (authorized == "?")
                        {
                            // if previous token is not '\n' error
                            if (!tokens.empty() && tokens.back().type != Token::Type::NewLine)
                                throw std::invalid_argument("Invalid '?' position at line " + std::to_string(line + 1));
                            type = Token::Type::Query;
                        }
                        else if (authorized == "(" || authorized == ")")
                            type = Token::Type::Parenthese;
                        else if (authorized == "!")
                            type = Token::Type::Unary;
                        else
                            type = Token::Type::Operator;
                        
                        tokens.push_back(Token(authorized, line, type));
                        i += authorized.length() - 1;
                    }
                    find_token = true;
                    break;
                }
            }
            if (!find_token)
                throw std::invalid_argument("Invalid character: " + string(1, input[i]) + " at line " + std::to_string(line + 1));
            find_token = false;
        }
    }
    return tokens;
}