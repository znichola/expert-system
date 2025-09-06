#ifndef EXPRESSION_HPP
# define EXPRESSION_HPP

# include <string>
# include <memory>
# include <variant>
# include <vector>

/*
 * The funcamental issue with this shit is :
 * When the compiler tries to figure out the size of Expr, it needs to know 
 * the size of Not. But Not contains an Expr, so it needs to know the size 
 * of Expr... infinite recursion!
 *
 * The solution is something to give it a constant size on the stack, we could 
 * use pointer, but who whant to deal with that headache. We can also use an
 * std::vector the container can manager the pointer for us.
 * */

struct Expr;

using ExprBoxed = std::vector<Expr>;

struct Var {
    explicit Var(char v);
    Var() = delete;
    char value() const;
private:
    char _v; // A B C ..
};


struct Not {
    explicit Not(Expr c);
    Not() = delete;
    Expr child() const;
private:
    ExprBoxed _v;
};


struct And {
    explicit And(Expr l, Expr r);
    And() = delete;
    Expr lhs() const;
    Expr rhs() const;
private:
    ExprBoxed _v;
};


struct Or {
    explicit Or(Expr l, Expr r);
    Or() = delete;
    Expr lhs() const;
    Expr rhs() const;
private:
    ExprBoxed _v;
};


struct Xor {
    explicit Xor(Expr l, Expr r);
    Xor() = delete;
    Expr lhs() const;
    Expr rhs() const;
private:
    ExprBoxed _v;
};


struct Imply {
    explicit Imply(Expr l, Expr r);
    Imply() = delete;
    Expr lhs() const;
    Expr rhs() const;
private:
    ExprBoxed _v;
};


struct Iff {
    explicit Iff(Expr l, Expr r);
    Iff() = delete;
    Expr lhs() const;
    Expr rhs() const;
private:
    ExprBoxed _v;
};


struct Expr : std::variant<Var, Not, And, Or, Xor, Imply, Iff> {
    using std::variant<Var, Not, And, Or, Xor, Imply, Iff>::variant;
};


struct Printer {
    std::string operator()(const Var &v) const
        { return std::string(1, v.value()); }
    std::string operator()(const Not &n) const
        { return "!" + std::visit(*this, n.child()); }
    std::string operator()(const And &n) const
        { return "(" + std::visit(*this, n.lhs()) + "+" + std::visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Or &n) const
        { return "(" + std::visit(*this, n.lhs()) + "|" + std::visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Xor &n) const
        { return "(" + std::visit(*this, n.lhs()) + "^" + std::visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Imply &n) const
        { return "(" + std::visit(*this, n.lhs()) + "=>" + std::visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Iff &n) const
        { return "(" + std::visit(*this, n.lhs()) + "<=>" + std::visit(*this, n.rhs()) + ")"; }
};



#endif /* EXPRESSION_HPP */

