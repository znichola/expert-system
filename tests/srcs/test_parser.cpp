#include <iostream>

#include "expert-system.hpp"
# include "parser.hpp"

int main() {
    std::cout << "Parsing tests" << std::endl; 
    std::string input = "A + (B | C) => D";
    try {
        vector<Token> tokens = tokenizer(input);
        Parser parser{0, tokens};
        auto foo = parser.parseExpr();
        if (foo)
            std::cout << "Parsed expression: " << foo->toString() << std::endl;
        else
            std::cout << "Failed to parse expression" << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Tokenizing error | " << e.what() << std::endl;
        return 1;
    }
}
