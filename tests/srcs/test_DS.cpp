#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "expert-system.hpp"

int main() {

//     Expr e = make_xor(
//             make_and(make_var('A'), make_not(make_var('B'))),
//             make_and(make_var('C'), make_var('D')));
// 
//     std::cout << "Test expression:"
//         << std::endl
//         << std::visit(Printer{}, e)
//         << std::endl;

    Expr e = Imply(Or(Var('A'), And(Var('B'), Var('C'))), Var('D'));

    std::cout << "Test expression:"
     << std::endl
     << std::visit(Printer{}, e)
     << std::endl;

    std::cout << "Test formal logic printer:"
     << std::endl
     << std::visit(PrinterFormalLogic{}, e)
     << std::endl;

    return 0;
}
