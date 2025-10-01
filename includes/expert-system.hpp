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
# include <set>
# include "expression.hpp"

using std::string;
using std::vector;

// tokenizer returns a vector of Tokens
struct Token {
    enum class Type {
        Variable,  // A B C ...
        Operator,  // <=> => + | ^
        Unary,     // !
        Fact,      // =
        Query,     // ?
        Parenthese,// ( )
        Comment,   // # to end of line
        NewLine,   // \n
    };
    string  token_list;
    size_t  line_number = 0;
    Type    type;

    Token() = delete;
    Token(const string &token_list, size_t line_number, Type type)
        : token_list(token_list), line_number(line_number), type(type) {}
    // whatever the token needs
};


// parseRules returns a vector of Rules
struct Rule {
    const Expr expr;
    size_t line_number = -1;
    int index = -1;
    std::string comment;
    const std::string id;

    // id of facts that are in the antecedent (premis) of the rule (lhs)
    std::vector<char> antecedent_facts;
    // TODO change this to unorderd_set!

    // id of facts that are in the consequent (conclusion) of the rule (rhs)
    std::vector<char> consequent_facts;

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
    const size_t line_number = -1;
    std::string comment;
    const char id;

    // ids of the rules this fact appears antecedent (premis) (lhs)
    std::vector<std::string> antecedent_rules;

    // ids of the rules this fact appears consequent (conclusion) (rhs)
    std::vector<std::string> consequent_rules;

    // no no-value construction, no invalid fact states
    Fact() = delete;

    // Construct a deduced facts, it has no position or comment 
    Fact(char label, State state) : label(label), state(state), id(label) {}

    // Construct a fact from inpupt data, with a comment and line number
    Fact(char label, State state, int line_number, const std::string &comment)
        : label(label), state(state), line_number(line_number),
            comment(comment), id(label) {}

    std::string toString() const {
        return std::string(1, label) + ":" + (
                state == State::True  ? "True" : 
                state == State::False ? "False" : "Undetermined");
    }
};

inline std::ostream& operator<<(std::ostream& os, const Fact& f) {
    os << f.toString();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Fact::State s) {
    os << (s == Fact::State::True ? "True" :
            s == Fact::State::False ? "False" : "Undetermined");
    return os;
}

// parseQueries returns a vector of Query
struct Query {
    const char label;
    const size_t line_number = -1;
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
    std::set<char> solving_stack; // Add this for cycle detection
//    FactMap  questFacts; // facts for which a search is already launched

    void addFact(const Fact &fact);

    // add rule implicitly will also add relevant facts
    void addRule(const Rule &rule);

    std::string toString() const;
    std::string toDot() const;
    std::vector<char> trueFacts() const;

    // These two functions are mutually recursive
    Fact::State solveForFact(const char fact_id, bool isExplain = false);
    Fact::State solveRule(const std::string &rule_id, bool isExplain = false);

    void setExprVarsToState(const Expr &expr, const Fact::State state);

    Fact::State solveExpr(const Expr &expr, bool isExplain = false);

};

inline std::ostream& operator<<(std::ostream& os, const Digraph& g) {
    return os << g.toString();
}

Digraph makeDigraph(
        const std::vector<Fact> &facts,
        const std::vector<Rule> &rules
    );





/* solver.cpp */
struct Foo {};
Foo solve(const std::vector<Rule> &rules,
        const std::vector<Fact> &facts,
        const Query &query);

/* tokenize.cpp */
vector<Token> tokenizer(string input);

/* parser.cpp */
struct Parsing
{
    size_t index;
    Expr lhs;
};
std::vector<Fact> parseFacts(const vector<Token> &input);
std::vector<Query> parseQueries(const vector<Token> &input);



#endif /* EXPERT_SYSTEM_HPP */
