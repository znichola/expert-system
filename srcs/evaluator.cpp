#include <string>
#include <memory>
#include <variant>

#include "expert-system.hpp"

Var::Var(char v) : value(v) {}

Not::Not(Expr c) : child(std::make_unique<Expr>(std::move(c))) {}

And::And(Expr l, Expr r) : lhs(std::make_unique<Expr>(std::move(l))),
    rhs(std::make_unique<Expr>(std::move(r))) {}

Or::Or(Expr l, Expr r) : lhs(std::make_unique<Expr>(std::move(l))),
    rhs(std::make_unique<Expr>(std::move(r))) {}

Xor::Xor(Expr l, Expr r) : lhs(std::make_unique<Expr>(std::move(l))),
    rhs(std::make_unique<Expr>(std::move(r))) {}

Imply::Imply(Expr l, Expr r) : lhs(std::make_unique<Expr>(std::move(l))),
    rhs(std::make_unique<Expr>(std::move(r))) {}

Iff::Iff(Expr l, Expr r) : lhs(std::make_unique<Expr>(std::move(l))),
    rhs(std::make_unique<Expr>(std::move(r))) {}


Expr make_var(char v) { return Var{v}; }
Expr make_not(Expr c) { return Not{std::move(c)}; }
Expr make_and(Expr l, Expr r) { return And{std::move(l), std::move(r)}; }
Expr make_or(Expr l, Expr r) { return Or{std::move(l), std::move(r)}; }
Expr make_xor(Expr l, Expr r) { return Xor{std::move(l), std::move(r)}; }
Expr make_imply(Expr l, Expr r) { return Imply{std::move(l), std::move(r)}; }
Expr make_iff(Expr l, Expr r) { return Iff{std::move(l), std::move(r)}; }


