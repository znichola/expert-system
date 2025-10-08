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
    std::unordered_map<char, Fact::State> expected;  // expected results for queries
};

// Helper to run one test
void runTest(const Test &t) {


    auto tokens = tokenizer(t.ruleSet);
    auto [rules, facts, queries] = parseTokens(tokens);

    Digraph digraph = makeDigraph(facts, rules);
    digraph.applyWorldAssumption(false);

    for (const auto &[label, expectedState] : t.expected) {
        auto res = digraph.solveForFact(label);

        if (res != expectedState) {
            cout << "Running test with ruleset:\n" << t.ruleSet << endl
                 << "Query " << label << " got " << res
                 << " (expected " << expectedState << ")\n"
                 << RED << "KO " << RESET << label << endl;
        } else {
            cout << GREEN << "OK" << RESET << endl;
        }
    }

    cout << "--------------------------------------\n";
}

int main() {
    cout << "Testing solver" << endl;

    std::vector<Test> tests = {
        {
            "A=>B\n=A\n?B",
            { {'B', Fact::State::True} }
        },
        {
            "A|B=>C\n=A\n?C",
            { {'C', Fact::State::True} }
        },
        {
            "A|B=>C\n=B\n?C",
            { {'C', Fact::State::True} }
        },
        {
            "C+E=>F\nH+S=>K\nF=>G\nK=>Y\n=CE\n?G",
            { {'G', Fact::State::True} }
        },
        {
            "A=>B|C\n=A\n?C", // Undetermined example from the docs
            { {'B', Fact::State::Undetermined}, {'C', Fact::State::Undetermined}}
        },
        {
            "A|F=>B\n=A\n?F", // Closed world, F is not part of any implicaiton solution
            { {'F', Fact::State::False}}
        },
        {
            "A=>B\n=A\n?F", // Closed world, should be false
            { {'F', Fact::State::False}}
        },
        {
            "!A=>B\n=A\n?B", // should be False, because closed world,
                             // no antecedent is true that
                             // could prove/disprove/undecide the fact, is this correct?
            { {'B', Fact::State::False} }
        },
        {
            "A|F=>B\n=A\n?F", // Closed world, should be false
            { {'F', Fact::State::False}}
        },
        {
            "A=>B^C\n=A\n?B", // 
            { {'B', Fact::State::Undetermined}}
        },
        {
            "A=>!B\n=A\n?B", // Netation in conclusion
            { {'B', Fact::State::False}}
        },
        {
            "A=>B^C\nA=>!B\n=A\n?C", // Xor in conclusion,
                                     // C must be true, because A is true and B is false
            { {'A', Fact::State::True}, {'B', Fact::State::False}, {'C', Fact::State::True}}
        }
    };

    for (const auto &t : tests) {
        runTest(t);
    }
}
