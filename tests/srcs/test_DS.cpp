#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "expert-system.hpp"

using std::cout, std::cerr, std::endl;

void testExprValidation();

int main() {

//     Expr e = make_xor(
//             make_and(make_var('A'), make_not(make_var('B'))),
//             make_and(make_var('C'), make_var('D')));
// 
//     std::cout << "Test expression:"
//         << std::endl
//         << std::visit(Printer{}, e)
//         << std::endl;

    cout << "Test foo" << endl;
    


    return 0;

    Expr e = Imply(Or(Var('A'), And(Var('B'), Var('C'))), Var('D'));

    cout << "Test expression:"
     << endl
     << std::visit(Printer{}, e)
     << endl;

    cout << "Test formal logic printer:"
     << endl
     << std::visit(PrinterFormalLogic{}, e)
     << endl;

    testExprValidation();

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

