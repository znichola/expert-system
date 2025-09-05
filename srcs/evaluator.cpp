#include <string>
#include <memory>
#include <variant>

#include "expert-system.hpp"

enum class UnaryOp { Not };

enum class BinaryOp { And, Or, Xor, Implies, Iff };

struct Expr;

struct Value {
    std::string value;
};

struct Unary {
    UnaryOp op;
    std::unique_ptr<Expr> a;
};

struct Binary {
    BinaryOp  op;
    std::unique_ptr<Expr> a;
    std::unique_ptr<Expr> b;
};

struct Expr {
    std::variant<Value, Unary, Binary> node;
};

