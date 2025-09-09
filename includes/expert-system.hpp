#ifndef EXPERT_SYSTEM_HPP
# define EXPERT_SYSTEM_HPP

# include <ostream>
# include <string>

# include "expression.hpp"

// tokenizer returns a vector of Tokens
struct Token {
    // whatever the token needs
};


// parseRules returns a vector of Rules
struct Rule {
    // A rule is one line in the input file
    // whatever a rule needs, AST etc..
    Expr expr;
    int line_number = -1;
    int index = -1;

    std::string comment;
    Rule() = delete;
    explicit Rule(const Expr &expr) : expr(expr) {};
    Rule(const Expr &expr, int line_number, std::string comment) 
        : expr(expr), line_number(line_number), comment(comment) {}

    std::string toString() const {
        return ("Rule #" + std::to_string(line_number) 
                + " is " + std::visit(Printer{}, expr));
    }
};

inline std::ostream& operator<<(std::ostream& os, const Rule& r) {
    os << r.toString();
    return os;
}


// parseFacts returns a vector of Facts
struct Fact {
    enum class State {True, False, Undetermined};

    const char lable;
    State state = State::Undetermined;

    // the rules used for the deduction, if empy it's a base truth
    std::vector<size_t> reasoning; 

    // no no-value construction, no invalid fact states
    Fact() = delete;

    explicit Fact(char lable) : lable(lable) {}
    Fact(char lable, State state) : lable(lable), state(state) {}

    std::string toString() const {
        return std::string(1, lable) + " is " + (
                state == State::True  ? "true" : 
                state == State::False ? "false" : "undetermined");
    }
};

inline std::ostream& operator<<(std::ostream& os, const Fact& f) {
    os << f.toString();
    return os;
}

// parseQueries returns a vector of Query
struct Query {
    // A query is the result for a variable
};


#endif /* EXPERT_SYSTEM_HPP */
