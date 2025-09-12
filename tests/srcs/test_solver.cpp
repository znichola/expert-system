#include "expert-system.hpp"
#include "parser.hpp"

using std::cout, std::endl;


int main () {
    cout << "Testing solver" << endl;

    // using input files pass them through the whole system and check the output
    struct Test {
        string inputString;
        string expectedDot;
    };

    auto tokens = tokenizer("A=>B\n=A\n?B");
    auto [rules, facts, queries] = parseTokens(tokens);

    Digraph digraph = makeDigraph(facts, rules);

    for (const auto &q : queries) {
        auto res = digraph.solveForFact(q.label);
        cout << "Query " << q.label << " is " << res << std::endl;
    }
}
