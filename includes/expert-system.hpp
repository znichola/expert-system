#ifndef EXPERT_SYSTEM_HPP
# define EXPERT_SYSTEM_HPP

# include <ostream>
# include <string>
# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <stdexcept>
# include <unordered_map>

# include "expression.hpp"

using std::string;
using std::vector;

// tokenizer returns a vector of Tokens
struct Token {
    std::vector<std::pair<std::string, size_t>> token_list;
    // whatever the token needs
};


// parseRules returns a vector of Rules
struct Rule {
    const Expr expr;
    int line_number = -1;
    int index = -1;
    std::string comment;
    const std::string id;

    // id of facts that are in the antecedent (premis) of the rule
    std::vector<std::string> antecedent_facts;

    // id of facts that are in the consequent (conclusion) of the rule
    std::vector<std::string> consequest_facts;

    Rule() = delete;

    explicit Rule(const Expr &expr) : expr(expr), id(expr.toString()) {};

    Rule(const Expr &expr, int line_number, std::string comment)
        : expr(expr), line_number(line_number),
        comment(comment), id(expr.toString()) {}

    std::string toString() const {
        return std::visit(Printer{}, expr);
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
    std::string comment;
    const char id;

    // ids of the rules this fact appears antecedent (premis)
    std::vector<std::string> antecedent_rules;

    // ids of the rules this fact appears consequest (conclusion)
    std::vector<std::string> consequest_rules;

    // no no-value construction, no invalid fact states
    Fact() = delete;

    // Construct a deduced facts, it has no position or comment 
    Fact(char label, State state) : label(label), state(state), id(label) {}

    // Construct a fact from inpupt data, with a comment and line number
    Fact(char label, State state, int line_number, const std::string &comment)
        : label(label), state(state), line_number(line_number),
            comment(comment), id(label) {}

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
    std::string comment;

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


//////////////////////////////////////////////
// NODE STORE 
//
// Store for all nodes of the graph, the key is .id field
// of both structs, it's a hashMap so elements are unique


struct Digraph {
    using FactsMap = std::unordered_map<char, Fact>;
    using RulesMap = std::unordered_map<std::string, Rule>;

    FactsMap facts;
    RulesMap rules;

    bool addFact(const Fact &fact);

    // add rule implicitly will also add relevant facts
    bool addRule(const Rule &rule);

    std::string toString() const;
    std::vector<char> trueFacts() const;
    std::string generateDotFile() const;
};

inline std::ostream& operator<<(std::ostream& os, const Digraph& g) {
    return os << g.toString();
}

/* solver.cpp */
struct Foo {};
Foo solve(const std::vector<Rule> &rules,
        const std::vector<Fact> &facts,
        const Query &query);

/* tokenize.cpp */
Token tokenizer(std::string input);

/* parser.cpp */
std::vector<Fact> parseFacts(const Token &input);
std::vector<Rule> parseRules(const Token &input);
std::vector<Query> parseQueries(const Token &input);

#endif /* EXPERT_SYSTEM_HPP */
