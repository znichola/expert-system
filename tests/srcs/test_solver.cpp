#include "expert-system.hpp"
#include "parser.hpp"

#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define RESET   "\033[0m"

using std::cout;
using std::endl;

// Struct for a single test case
struct Test {
    std::string description;
    std::string ruleSet;                             // input rules/facts/queries
    std::unordered_map<char, Fact::State> expected;  // expected results for queries
};

// Helper to run one test
void runTest(const Test &t) {
    auto tokens = tokenizer(t.ruleSet);
    auto [rules, facts, queries] = parseTokens(tokens);

    Digraph digraph = makeDigraph(facts, rules);
    digraph.applyWorldAssumption(false);

    bool failed = false;

    for (const auto &[label, expectedState] : t.expected) {
        auto res = digraph.solveForFact(label);

        if (res != expectedState) {
            if (!failed) {
                cout << RED << "Test failed: " << RESET << t.description << endl;
                cout << "Ruleset:\n" << t.ruleSet << endl;
            }

            cout << "Query " << label << " got " << res
                 << " (expected " << expectedState << ")\n"
                 << RED << "KO " << RESET << label << endl;

            failed = true;
        }
    }

    if (!failed)
        cout << GREEN << "OK" << RESET << endl;

    cout << "--------------------------------------\n";
}

int main() {
    cout << "Testing solver" << endl;

    std::vector<Test> tests = {
        {
            "Simple implication",
            "A=>B\n=A\n?B",
            { {'B', Fact::State::True} }
        },
        {
            "OR in consequent: lhs as true",
            "A|B=>C\n=A\n?C",
            { {'C', Fact::State::True} }
        },
        {
            "OR in consequen: rhs as true",
            "A|B=>C\n=B\n?C",
            { {'C', Fact::State::True} }
        },
        {
            "Chained rules and ANDs",
            "C+E=>F\nH+S=>K\nF=>G\nK=>Y\n=CE\n?G",
            { {'G', Fact::State::True} }
        },
        {
            "Or in conclusion: Undetermined example from docs",
            "A=>B|C\n=A\n?C",
            { {'B', Fact::State::Undetermined}, {'C', Fact::State::Undetermined}}
        },
        {
            "Closed world assumption: F is not part of any implication therefore it should be false",
            "A|F=>B\n=A\n?F",
            { {'F', Fact::State::False}}
        },
        {
            "Closed world assumption with unused F",
            "A=>B\n=A\n?F",
            { {'F', Fact::State::False}}
        },
        {
            "Negation in rule: Should yield false becasue closed world and no antecedent \n\
             is true that could prove/disprove/undecide the fact, (is this correct?)",
            "!A=>B\n=A\n?B",
            { {'B', Fact::State::False} }
        },
        {
            "Negation in conclusion: A=>!B, B should be false",
            "A=>!B\n=A\n?B",
            { {'B', Fact::State::False}}
        },
        {
            "XOR in conclusion, with hls as true",
            "A=>B^C\n=AB\n?BC",
            { {'B', Fact::State::True}, {'C', Fact::State::False}}
        },
        {
            "XOR in conclusion with rhs as negative",
            "A=>B^C\nA=>!B\n=A\n?C",
            { {'A', Fact::State::True}, {'B', Fact::State::False}, {'C', Fact::State::True}}
        },
        {
            "XOR and NOT combined in conclusion",
            "A=>B^!C\n=AB\n?BC",
            { {'B', Fact::State::True}, {'C', Fact::State::True}}
        },
        {
            "AND in conclusion",
            "A=>B+C\n=A\n?BC",
            { {'B', Fact::State::True}, {'C', Fact::State::True}}
        },
        {
            "Compound AND in conclusion",
            "A=>B+C+D\n=A\n?BCD",
            { {'B', Fact::State::True}, {'C', Fact::State::True}, {'D', Fact::State::True}}
        },
        {
            "Combining AND with NOT in a conclusion",
            "A=>B+!C\n=A\n?BC",
            { {'B', Fact::State::True}, {'C', Fact::State::False}}
        },
        {
            "Or in conclusion, partial lhs knowledge it's false",
            "A=>B|C\nA=>!B\n=A\n?BC",
            { {'B', Fact::State::False}, {'C', Fact::State::True}}
        },
        {
            "Negated Or in conclusion",
            "A=>!(B|C)\n=A\n?BC",
            { {'B', Fact::State::False}, {'C', Fact::State::False}}
        },
        {
            "shouldWork 9 : Xor in conclusion",
            "A=>L^U\nL=>B\nU=>B\nB=>M\n=A\n?MLU",
            { {'M', Fact::State::False}, {'C', Fact::State::False}}
        },
        {
            "shouldWork 10 : Or in conclusion",
            "A=>L|U\nL=>B\nU=>B\nB=>M\n=A\n?MLU",
            { {'M', Fact::State::False}, {'C', Fact::State::False}}
        },
        {
            "shouldWork 15 : circular deps",
            "A=>B\nB=>C\nC=>D\nD=>A\n=Z\n?D",
            { {'D', Fact::State::False}}
        },
        {
            "Iff false lhs, rhs must also be false",
            "A=>!B\nB<=>C\n=A\n?C",
            { {'B', Fact::State::False}, {'C', Fact::State::False}}
        },
        {
            "Two Implies that loop should be equivilent to a iff",
            "A=>!B\nB=>C\nC=>B\n=A\n?C",
            { {'B', Fact::State::False}, {'C', Fact::State::False}}
        },
    };

    for (const auto &t : tests) {
        runTest(t);
    }
}
