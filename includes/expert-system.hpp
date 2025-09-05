#ifndef EXPERT_SYSTEM_HPP
# define EXPERT_SYSTEM_HPP

# include <string>
# include <memory>
# include <variant>

// tokenizer returns a vector of Tokens
struct Token {
    // whatever the token needs
};


// parseRules returns a vector of Rules
struct Rule {
    // A rule is one line in the input file
    // whatever a rule needs, AST etc..
};


// parseFacts returns a vector of Facts
struct Fact {
    // A fact is a statment about the value
    // of a variable from the input file
};


// parseQueries returns a vector of Query
struct Query {
    // A query is the result for a variable
};


// Expression Tree

struct Expr;

Expr make_var(char v);
Expr make_not(Expr c);
Expr make_and(Expr l, Expr r);
Expr make_or(Expr l, Expr r);
Expr make_xor(Expr l, Expr r);
Expr make_imply(Expr l, Expr r);
Expr make_iff(Expr l, Expr r);

struct Var {
    char value; // A B C ..
private:
    explicit Var(char v);
    friend Expr make_var(char v);
};

struct Not {
    std::unique_ptr<Expr> child;
private:
    explicit Not(Expr c);
    friend Expr make_not(Expr c);
};

struct And {
    std::unique_ptr<Expr> lhs, rhs;
private:
    explicit And(Expr l, Expr r);
    friend Expr make_and(Expr l, Expr r);
}; 

struct Or {
    std::unique_ptr<Expr> lhs, rhs;
private:
    explicit Or(Expr l, Expr r);
    friend Expr make_or(Expr l, Expr r);
}; 

struct Xor {
    std::unique_ptr<Expr> lhs, rhs;
private:
    explicit Xor(Expr l, Expr r);
    friend Expr make_xor(Expr l, Expr r);
}; 

struct Imply {
    std::unique_ptr<Expr> lhs, rhs;
private:
    explicit Imply(Expr l, Expr r);
    friend Expr make_imply(Expr l, Expr r);
}; 

struct Iff{
    std::unique_ptr<Expr> lhs, rhs;
private:
    explicit Iff(Expr l, Expr r);
    friend Expr make_iff(Expr l, Expr r);
};

struct Expr : std::variant<Var, Not, And, Or, Xor, Imply, Iff> {
   using std::variant<Var, Not, And, Or, Xor, Imply, Iff>::variant;
};

struct Printer {
    std::string operator()(const Var &v) const
        { return std::string(1, v.value); }
    std::string operator()(const Not &n) const
        { return "!" + std::visit(*this, *n.child); }
    std::string operator()(const And &n) const
        { return "(" + std::visit(*this, *n.lhs) + "+" + std::visit(*this, *n.rhs) + ")"; }
    std::string operator()(const Or &n) const
        { return "(" + std::visit(*this, *n.lhs) + "|" + std::visit(*this, *n.rhs) + ")"; }
    std::string operator()(const Xor &n) const
        { return "(" + std::visit(*this, *n.lhs) + "^" + std::visit(*this, *n.rhs) + ")"; }
    std::string operator()(const Imply &n) const
        { return "(" + std::visit(*this, *n.lhs) + "=>" + std::visit(*this, *n.rhs) + ")"; }
    std::string operator()(const Iff &n) const
        { return "(" + std::visit(*this, *n.lhs) + "<=>" + std::visit(*this, *n.rhs) + ")"; }
};

#endif /* EXPERT_SYSTEM_HPP */
