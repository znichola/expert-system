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

    SolveExpr solver;
    solver.digraph = digraph;

    Fact::State res = std::visit(solver, rules[0].expr);

    cout << "Result: " << (res == Fact::State::True ? "True" : res == Fact::State::False ? "False" : "Undetermined") << endl;
}