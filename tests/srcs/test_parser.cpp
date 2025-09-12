#include <iostream>
#include <string>
#include "expert-system.hpp"
#include "parser.hpp"

// ANSI color codes
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define RESET   "\033[0m"

int ko_count = 0;
int test_count = 0;

void test(const std::string& input, bool should_pass = true) {
    ++test_count;
    try {
        std::vector<Token> tokens = tokenizer(input);
        Parser parser{0, tokens};
        Expr expr = parser.parse();
        
        // take the expr and convert it back to string, the compare this to the input to check if test passed
        std::string expr_str = expr.toString();
        if (expr_str == input) {
            if (should_pass) {
                std::cout << "Input: " << input << " " << GREEN << "OK" << RESET << "\n";
            } else {
                ++ko_count;
                std::cout << "Input: " << input << " " << RED << "KO" << RESET << "\n";
                std::cout << "  Expected failure but passed\n";
            }
        } else {
            ++ko_count;
            std::cout << "Input: " << input << " " << RED << "KO" << RESET << "\n";
            std::cout << "  Expected: " << input << "\n";
            std::cout << "  Got:      " << expr_str << "\n";
        }
        
    } catch (std::exception &e) {
        if (!should_pass) {
            std::cout << "Input: " << input << " " << GREEN << "OK" << RESET << "\n";
        } else {
            ++ko_count;
            std::cout << "Input: " << input << " " << RED << "KO" << RESET << "\n";
            std::cout << "  Exception: " << e.what() << "\n";
        }
    }
}

int main() {
    std::cout << "Parsing tests\n";

    // test should be between parentheses
    test("A");
    test("(A+B)");
    test("(A|B)");
    test("((A+B)+C)");
    test("((A|B)+(C^D))");
    test("(((A|B)+(C^D))=>E)");
    test("(A=>B)");
    test("((A=>B)=>(C=>D))");
    test("((A=>B)<=>C)");
    test("((A+B)=>(!C|D))");
    test("!(A+B)");
    test("(A|!(H+(!J^L)))");
    test("(A|!(H+(J^!L)))");
    test("!!(!A+!!!B)#bfhbewjrhjer");

    std::cout << "\nOK's : " << test_count - ko_count << " / " << test_count << "\n";
}