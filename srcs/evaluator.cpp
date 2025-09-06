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


