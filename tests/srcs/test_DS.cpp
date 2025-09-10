#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "expert-system.hpp"

using std::cout, std::cerr, std::endl;

void testSocratiesRuleSet();
void testExprValidation();

int main() {

    cout << "Test foo" << endl;

    testSocratiesRuleSet();
    return 0;

    auto e = Imply(Or(Var('A'), And(Var('B'), Var('C'))), Var('D'));

    auto f = Fact('A', Fact::State::Undetermined);
    auto f1 = Fact('B', Fact::State::True);

    auto r = Rule(e, 42, "is the answer");

    auto q = Query('A');

    cout << e << endl << f << endl << r << endl;

    std::vector<Rule> rules = {r};
    std::vector<Fact> facts = {f, f1};

    cout << endl << endl << "####";
    (void)solve(rules, facts, q);
    cout << "####"<< endl << endl;

    return 0;
}


void testSocratiesRuleSet() {
    cout << "### Socraties is a man" << endl;
    std::vector<Rule> rules = {
        Rule(Imply(Var('M'), Var('D')), 3, "All men are mortal"),
    };
    std::vector<Fact> facts = {
        Fact('M', Fact::State::True, 6, "He is a man"),
    };

    auto query = Query('D', 9, "Is he mortal?");

    (void)solve(rules, facts, query);

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

