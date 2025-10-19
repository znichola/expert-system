#include "expert-system.hpp"
#include "parser.hpp"

#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define RESET   "\033[0m"

using std::cout;
using std::endl;

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

    const Expr::VarMap varMap({{'A', true}, {'B', false}, {'C', false}});

    for (const auto &[label, expectedState] : t.expected) {
        auto res = digraph.solveForFact(label);
        
        const auto expr = digraph.rules.begin()->second.expr;
        const auto ev = expr.booleanEvaluate(varMap);
        
        cout << "TEST:" << expr << " with [" << varMap
            << "] is " << (ev ? "true" : "false") << "\n";

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

void testBooleanExprEvaluator();

int main()
{
    cout << "Testing boolean evaluator" << endl;

    testBooleanExprEvaluator();

    // auto tokens = tokenizer("A=>B|G\nB=>C\nC=>D\nD=>A\n=A\nH=>K\nL=>H+K\n?D");
    auto tokens = tokenizer("A=>B\nB=>C\nC=>D\nD=>A\n=Z\n?D");
    auto [rules, facts, queries] = parseTokens(tokens);

    Digraph digraph = makeDigraph(facts, rules);
    Expr compiledExpr = digraph.compileExprForFact('C');
    cout << "" << compiledExpr << endl;

    auto res = digraph.boolMapEvaluate(compiledExpr);

    cout << "ALKSDJ\n" << res << endl;
}

void testBooleanExprEvaluator() {
    cout << "Test boolean expr evaluator" << endl;

    auto tokens = tokenizer("A=>B\n=A\n?B");
    auto [rules, facts, queries] = parseTokens(tokens);

    Digraph digraph = makeDigraph(facts, rules);
    digraph.applyWorldAssumption(false);

    const Expr::VarMap varMap({{'A', true}, {'B', true}});

        auto res = digraph.solveForFact('B');

        const auto expr = digraph.rules.begin()->second.expr;
        const auto ev = expr.booleanEvaluate(varMap);

        if (res == Fact::State::True && ev == true) {
            cout << GREEN << "OK" << RESET;
        } else {
            cout << RED << "KO" << RESET;
        }

        cout << " test:" << expr << " with [" << varMap
            << "] is " << (ev ? "true" : "false") << "\n";
        
}
 