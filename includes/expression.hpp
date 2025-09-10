#ifndef EXPRESSION_HPP
# define EXPRESSION_HPP

# include <string>
# include <memory>
# include <variant>
# include <vector>
# include <optional>

/*
 * The funcamental issue with this shit is :
 * When the compiler tries to figure out the size of Expr, it needs to know 
 * the size of Not. But Not contains an Expr, so it needs to know the size 
 * of Expr... infinite recursion!
 *
 * The solution is something to give it a constant size on the stack, we could 
 * use pointer, but who wants to deal with that headache. We can also use an
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

struct ValueGetter;

struct Expr : std::variant<Var, Not, And, Or, Xor, Imply, Iff> {
    using std::variant<Var, Not, And, Or, Xor, Imply, Iff>::variant;
    Expr() = delete; // want to remove the defualt no args constuctions
                     // it's a completly invalid state!
    bool isValidRule() const;
    bool isSimpleExpr() const;
    ValueGetter getValues() const;
    bool containes(const Var &var) const;
    std::string toString() const;
};

using std::visit;

struct Printer {
    std::string operator()(const Var &v) const
        { return std::string(1, v.value()); }
    std::string operator()(const Not &n) const
        { return "!" + visit(*this, n.child()); }
    std::string operator()(const And &n) const
        { return "(" + visit(*this, n.lhs()) + "+" + visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Or &n) const
        { return "(" + visit(*this, n.lhs()) + "|" + visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Xor &n) const
        { return "(" + visit(*this, n.lhs()) + "^" + visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Imply &n) const
        { return "(" + visit(*this, n.lhs()) + "=>" + visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Iff &n) const
        { return "(" + visit(*this, n.lhs()) + "<=>" + visit(*this, n.rhs()) + ")"; }
};

inline std::ostream& operator<<(std::ostream& os, const Expr& e) {
    os << visit(Printer{}, e);
    return os;
}

inline std::string Expr::toString() const {
    return visit(Printer{}, *this);
}

struct PrinterFormalLogic {
    std::string operator()(const Var &v) const
        { return std::string(1, v.value()); }
    std::string operator()(const Not &n) const
        { return "¬" + visit(*this, n.child()); }
    std::string operator()(const And &n) const
        { return "(" + visit(*this, n.lhs()) + " ∧ " + visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Or &n) const
        { return "(" + visit(*this, n.lhs()) + " ∨ " + visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Xor &n) const
        { return "(" + visit(*this, n.lhs()) + " ⊕ " + visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Imply &n) const
        { return "(" + visit(*this, n.lhs()) + " ⇒ " + visit(*this, n.rhs()) + ")"; }
    std::string operator()(const Iff &n) const
        { return "(" + visit(*this, n.lhs()) + " ⇔ " + visit(*this, n.rhs()) + ")"; }
};


// This is only half implemented
struct PrinterExplenation {
    std::string operator()(const Var &v) const
    { return std::string(1, v.value()); }
    std::string operator()(const Not &n) const
    { return "not " + visit(*this, n.child()); }
    std::string operator()(const And &n) const
    { return visit(*this, n.lhs()) + " and " + visit(*this, n.rhs()); }
    std::string operator()(const Or &n) const
    { return visit(*this, n.lhs()) + " or " + visit(*this, n.rhs()); }
    std::string operator()(const Xor &n) const
    { return visit(*this, n.lhs()) + " xor " + visit(*this, n.rhs()); }
    std::string operator()(const Imply &n) const
    { return visit(*this, n.lhs()) + " implies " + visit(*this, n.rhs()); }
    std::string operator()(const Iff &n) const
    { return visit(*this, n.lhs()) + " if-and-only-if " + visit(*this, n.rhs()); }
};

// used to unpack the variant and get lhs & rhs values if they exist
struct ValueGetter {
    std::optional<Expr> child, lhs, rhs;
    std::optional<char> value;

    void operator()(const And &n)   { lhs = n.lhs(); rhs = n.rhs(); }
    void operator()(const Or &n)    { lhs = n.lhs(); rhs = n.rhs(); }
    void operator()(const Xor &n)   { lhs = n.lhs(); rhs = n.rhs(); }
    void operator()(const Imply &n) { lhs = n.lhs(); rhs = n.rhs(); }
    void operator()(const Iff &n)   { lhs = n.lhs(); rhs = n.rhs(); }

    void operator()(const Not &n) { child = n.child(); }
    void operator()(const Var &v) { value = v.value(); }
};

inline ValueGetter Expr::getValues() const {
    ValueGetter g;
    visit(g, *this);
    return g;
}

# include <iostream>

inline bool Expr::containes(const Var &var) const {
    auto g = getValues();
    if (g.value) {
        return g.value == var.value();
    }
    if (g.child && g.child.value().containes(var)) {
        return true;
    }
    if (g.lhs && g.lhs.value().containes(var)) {
        return true;
    }
    if (g.rhs && g.rhs.value().containes(var)) {
        return true;
    }
    return false;
}

//////////////////////////////////////////
/// FUNCITONS
/// 
/// and usefull stuffs

#endif /* EXPRESSION_HPP */

