#include <string>
#include <memory>
#include <variant>

#include "expression.hpp"

  Var::Var  (char v) : _v(v) {}
  Not::Not  (const Expr c) : _v{std::move(c)} {}
  And::And  (const Expr l, const Expr r) : _v{std::move(l), std::move(r)} {}
   Or::Or   (const Expr l, const Expr r) : _v{std::move(l), std::move(r)} {}
  Xor::Xor  (const Expr l, const Expr r) : _v{std::move(l), std::move(r)} {}
Imply::Imply(const Expr l, const Expr r) : _v{std::move(l), std::move(r)} {}
  Iff::Iff  (const Expr l, const Expr r) : _v{std::move(l), std::move(r)} {}

char Var::value() const { return _v; }
Expr Not::child() const { return _v[0]; }
Expr And::lhs()   const { return _v[0]; } Expr And::rhs()   const { return _v[1]; }
Expr Or::lhs()    const { return _v[0]; } Expr Or::rhs()    const { return _v[1]; }
Expr Xor::lhs()   const { return _v[0]; } Expr Xor::rhs()   const { return _v[1]; }
Expr Imply::lhs() const { return _v[0]; } Expr Imply::rhs() const { return _v[1]; }
Expr Iff::lhs()   const { return _v[0]; } Expr Iff::rhs()   const { return _v[1]; }

# include <iostream>

bool Expr::isValidRule() const {

    // has imply or Iff at root
    if (!std::holds_alternative<Imply>(*this)
            && !std::holds_alternative<Iff>(*this)) {
        return false;
    }

    // check rhs is a "simple expression" just a value or an "and", "or", "xor" 
    ValueGetter g; std::visit(g, *this);
    if (g.rhs) {
        return g.rhs.value().isSimpleExpr();
    }

    return true;
}

bool Expr::isSimpleExpr() const {
    if (std::holds_alternative<Var>(*this)) {
        return true;
    }

    if (std::holds_alternative<Not>(*this)) {
        const auto &op = std::get<Not>(*this);
        return std::holds_alternative<Var>(op.child());
    }

    if (std::holds_alternative<And>(*this)) {
        const auto &op = std::get<And>(*this);
        return std::holds_alternative<Var>(op.lhs())
            && std::holds_alternative<Var>(op.rhs());
    }
    if (std::holds_alternative<Or>(*this)) {
        const auto &op = std::get<Or>(*this);
        return std::holds_alternative<Var>(op.lhs())
            && std::holds_alternative<Var>(op.rhs());
    }
    if (std::holds_alternative<Xor>(*this)) {
        const auto &op = std::get<Xor>(*this);
        return std::holds_alternative<Var>(op.lhs())
            && std::holds_alternative<Var>(op.rhs());
    }

    return false;
}
