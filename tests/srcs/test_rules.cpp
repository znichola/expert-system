#include "expert-system.hpp"
#include "parser.hpp"


struct Test {
    string inputFile;
    string expectedDot;
};

bool runTest (const Test& test);

int main()
{
    std::vector<Rule> rules;
    std::vector<Fact> facts;
    std::vector<Query> queries;


    vector<Test> fileParsingValidation = {
        // paste a string with multiple lines and newlines 
        {{R"(# Fritz the frog

# Rules
C + E => F # if X croaks and eats flies, then X is frog
H + S => K # if X chirps and sings, then X is a canary
F => G     # if X is a frog, then X is green
K => Y     # if X is a canary, then X is yellow

# Facts about Fritz
=CE # croaks, eats flies

# Query, or question being asked
?G # Is Fritz green?)"},
        {R"(
        
            )"}}
    };

    for (const auto& test : fileParsingValidation) {
        if (!runTest(test)) {
            std::cerr << "KO" << std::endl;
        } else {
            std::cout << "OK" << std::endl;
        }
    }
}


bool runTest (const Test& test) {
    std::vector<Token> tokens;
    try {
        tokens = tokenizer(test.inputFile);
    } catch (const std::exception& e) {
        std::cerr << "Tokenizer error: " << e.what() << std::endl;
        return false;
    }

    std::tuple<std::vector<Rule>, std::vector<Fact>, std::vector<Query>> parsedData;
    try {
        parsedData = parseTokens(tokens);
    } catch (const std::exception& e) {
        std::cerr << "ParseTokens error: " << e.what() << std::endl;
        return false;
    }

    auto [rules, facts, queries] = parsedData;

    Digraph digraph;
    try {
        digraph = makeDigraph(facts, rules);
    } catch (const std::exception& e) {
        std::cerr << "MakeDigraph error: " << e.what() << std::endl;
        return false;
    }

    std::string dot = digraph.toDot();
    if (dot != test.expectedDot) {
        std::cerr << "Expected:\n" << test.expectedDot << "\nGot:\n" << dot << std::endl;
        return false;
    }
    return true;
};
