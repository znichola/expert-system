#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "expert-system.hpp"

using std::cout, std::cerr, std::endl;

void testExprValidation();

int main() {

    cout << "Test foo" << endl;

    auto e = Imply(Or(Var('A'), And(Var('B'), Var('C'))), Var('D'));

    auto f = Fact('A');

    auto r = Rule(e, 42, "is the answer");

    cout << e << endl << f << endl << r << endl;

    return 0;
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

