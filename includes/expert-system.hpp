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
    Expr expr;
    int line_number = -1;
    int index = -1;
    const std::string comment;

    Rule() = delete;

    explicit Rule(const Expr &expr) : expr(expr) {};

    Rule(const Expr &expr, int line_number, std::string comment) 
        : expr(expr), line_number(line_number), comment(comment) {}

    std::string toString() const {
        return ("Rule #" + std::to_string(line_number) 
                + "  " + std::visit(Printer{}, expr));
    }
};

inline std::ostream& operator<<(std::ostream& os, const Rule& r) {
    os << r.toString();
    return os;
}


// parseFacts returns a vector of Facts
struct Fact {
    enum class State {True, False, Undetermined};

    const char label;
    State state = State::Undetermined;
    const int line_number = -1;
    const std::string comment;

    // the rules used for the deduction, if empy it's a base truth
    std::vector<int> reasoning;

    // no no-value construction, no invalid fact states
    Fact() = delete;

    // Construct a deduced facts, it has no position or comment 
    Fact(char label, State state) : label(label), state(state) {}

    // Construct a fact from inpupt data, with a comment and line number
    Fact(char label, State state, int line_number, const std::string &comment)
        : label(label), state(state), line_number(line_number), comment(comment)
        {}

    std::string toString() const {
        return std::string(1, label) + " is " + (
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
    const char label;
    const int line_number = -1;
    const std::string comment;

    Query() = delete;

    // Construct an induced query, no comment or history
    explicit Query(char label) : label(label) {};

    // Construct a fact from inpupt data, with a comment and line number
    Query(char label, int line_number, const std::string &comment)
        : label(label), line_number(line_number), comment(comment) {}

    std::string toString() const {
        return std::string(1, label);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Query& q) {
    return os << q.toString();
}


/* solver.cpp */
struct Foo {};
Foo solve(const std::vector<Rule> &rules,
        const std::vector<Fact> &facts,
        const Query &query);

#endif /* EXPERT_SYSTEM_HPP */
