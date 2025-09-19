#include "expert-system.hpp"
#include "parser.hpp"

#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define RESET   "\033[0m"

using std::cout;
using std::endl;

// Struct for a single test case
struct Test {
    std::string ruleSet;                            // input rules/facts/queries
    std::unordered_map<char, Fact::State> expected; // expected results for queries
};

// Helper to run one test
void runTest(const Test &t) {
    cout << "Running test with ruleset:\n" << t.ruleSet << endl;

    auto tokens = tokenizer(t.ruleSet);
    auto [rules, facts, queries] = parseTokens(tokens);

    Digraph digraph = makeDigraph(facts, rules);

    for (const auto &[label, expectedState] : t.expected) {
        auto res = digraph.solveForFact(label);

        cout << "Query " << label << " got " << res
             << " (expected " << expectedState << ")\n";

        if (res != expectedState) {
            cout << RED << "KO " << RESET << label << endl;
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
            "!A=>B\n=A\n?B",
            { {'B', Fact::State::False} }
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
            "A=>B\nB=>C\nC=>D\nD=>A\n=A\n?D",
            { {'D', Fact::State::True} }
        },
        {
            "A=>B\nB=>C\nC=>D\nD=>A\n=Z\n?D",
            { {'D', Fact::State::Undetermined} }
        }

    };

    for (const auto &t : tests) {
        runTest(t);
    }
}
