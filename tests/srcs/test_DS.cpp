#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "expert-system.hpp"

using std::cout, std::cerr, std::endl;

void testExprValidation();
void testExprContaines();
void testDigraph();

void testSocratiesRuleSet();

int main() {
    cout << "DS" << endl;

    auto foo = And(Var('A'), Var('B'));


    cout << "YAYA" << endl;

    testExprValidation();
    testExprContaines();
    testDigraph();

    testSocratiesRuleSet();
}


void testDigraph() {
    cout << "Digraph struct" << endl;

    Digraph digraph;

    auto f = Fact('A', Fact::State::True);
    auto r = Rule(Imply(And(Var('A'), Xor(Var('B'), Var('C'))), Var('R')));

    digraph.addFact(f);
    digraph.addRule(r);

    cout << digraph << endl;
}


void testSocratiesRuleSet() {
    cout << "### Socraties is a man" << endl;
    std::vector<Rule> rules = {
        Rule(Imply(Var('M'), And(Var('D'), Var('G'))), 3, "All men are mortal"),
    };
    std::vector<Fact> facts = {
        Fact('M', Fact::State::True, 6, "He is a man"),
    };

    auto query = Query('D', 9, "Is he mortal?");

    auto g = rules[0].expr.getValues();

    cout << "RESS :" <<  rules[0].expr.containes(Var('G')) << endl;

    (void)solve(rules, facts, query);

}


void testExprContaines() {
    cout << "Expr.containes()" << endl;
    struct Test {
        bool expected;
        char input;
        Expr expr;
    };
    std::vector<Test> simpleExprValidation = {
        { true, 'A', Imply(Var('A'), Var('B')) },
        { true, 'Z', Iff(Var('X'), And(Var('Y'), Var('Z'))) },
        { false, 'X', And(Var('A'), Var('B')) },
        { true, 'Y', Imply(Var('A'), Xor(Not(Var('B')), Not(Var('Y')))) },
        { false, 'H', And(Var('A'), Imply(Var('K'), Var('Y'))) }
    };

    for (const auto &t : simpleExprValidation) {
        bool result = t.expr.containes(Var(t.input));
        if (result == t.expected) {
            cout << "OK" << endl;
        } else {
            cerr << "KO: " << t.expr << " does not containe " << t.input
                 << std::boolalpha << t.expected << ")" << endl;
        }
    }
}


void testExprValidation() {
    cout << "Expression validations" << endl;

    std::vector<std::pair<bool, Expr>> simpleExprValidation = {
        { true,  Imply(Var('A'), Var('B')) },
        { true,  Iff(Var('X'), And(Var('Y'), Var('Z'))) },
        { false, And(Var('A'), Var('B')) },
        { true, Imply(Var('A'), Not(Var('B'))) },
        { false, And(Var('A'), Imply(Var('K'), Var('Y'))) }
    };

    for (const auto &t : simpleExprValidation) {
        bool expected = t.first;
        const Expr &expr = t.second;


        bool result = expr.isValidRule();
        if (result == expected) {
            cout << "OK" << endl;
        } else {
            cerr << "KO: " << expr << " (expected "
                 << std::boolalpha << expected << ")" << endl;
        }
    }
}

