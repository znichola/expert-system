#include "expert-system.hpp"
#include "parser.hpp"

#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define RESET   "\033[0m"

using std::cout;
using std::endl;

// Struct for a single test case
struct Test {
    std::string ruleSet;                             // input rules/facts/queries
    std::vector<std::string> expected;  // expected results for queries
};

// This test will only test the tokenizer, not the solver
// it will compare the output of the tokenizer to the expected output
// Helper to run one test
void runTest(const Test &t) {
    bool all_ok = true;
    cout << "Running test with ruleset:\n" << t.ruleSet << endl;
    
    auto tokens = tokenizer(t.ruleSet);
    for (size_t i = 0; i < tokens.size(); i++) {
        if (i >= t.expected.size() || tokens[i].token_list != t.expected[i]) {
            cout << RED << "KO " << RESET << tokens[i].token_list << endl;
            if (i < t.expected.size()) {
                cout << "  Expected: " << t.expected[i] << endl;
            } else {
                cout << "  Expected: <no token>" << endl;
            }
            all_ok = false;
        }
    }
    if (tokens.size() == t.expected.size() && all_ok) {
        cout << GREEN << "OK" << RESET << endl;
    } else if (tokens.size() != t.expected.size()) {
        cout << RED << "KO " << RESET << "Number of tokens mismatch" << endl;
        cout << "  Expected: " << t.expected.size() << endl;
        cout << "  Got:      " << tokens.size() << endl;
    }
    else if (all_ok) {
        cout << GREEN << "OK" << RESET << endl;
    }
    else {
        cout << RED << "KO" << RESET << endl;
    }

    cout << "--------------------------------------\n";
}

int main() {
    cout << "Testing solver" << endl;

    std::vector<Test> tests = {
        {
            "A=>!B#hello\n=A\n?B",
            { "A","=>", "!", "B", "#hello", "\n", "=", "A", "\n", "?", "B" }
        },
        {
            "!A<=>B|C^(N+!G)#hello\n=A\n?B",
            { "!", "A", "<=>", "B", "|", "C", "^", "(", "N", "+", "!", "G", ")", "#hello", "\n", "=", "A", "\n", "?", "B" }
        }
    };

    for (const auto &t : tests) {
        runTest(t);
    }
}
