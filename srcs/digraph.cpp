#include <set>

#include "expert-system.hpp"

std::string Digraph::toString() const {
    std::string res = "=== Digraph State ===\n";

    res += "Facts (" + std::to_string(facts.size()) + "):\n";
    for (const auto &kv : facts) {
        res += "- " + kv.second.toString() + "\n";
    }

    res += "Rules (" + std::to_string(rules.size()) + "):\n";
    for (const auto &kv : rules) {
        res += "- " + kv.second.toString() + "\n";
    }

    return res + "=====================";
}


std::string Digraph::toDot() const {
    std::stringstream ss;

    ss << "strict digraph {\n";

    for (const auto &kv : facts) {
        auto fact = kv.second;

        if (fact.consequent_rules.size() == 0) {
            ss << "  " << kv.first << "\n";
        } else for (const auto &r: fact.consequent_rules) {
            ss << "  " << kv.first << " -> \"" << r << "\"\n";
            //ss << "  \"" << r << "\" -> " << kv.first << "\n";
           (void)r;
        }

        if (fact.antecedent_rules.size() == 0) {
            //ss << "  " << kv.first << "\n";
        } else for (const auto &r: fact.antecedent_rules) {
            //ss << "  " << kv.first << " -> \"" << r << "\"\n";
            //ss <<  " \"" << r << "\" -> " << kv.first << "\n";
           (void)r;
        }
    }
    ss << "\n\n";

    for (const auto &kv : rules) {
        auto rule = kv.second;

        if (rule.consequent_facts.size() == 0) {
            //ss << "  \"" << kv.first << "\"\n";
        } else for (const auto &f: rule.consequent_facts) {
            //ss << "  \"" << kv.first << "\" -> " << f << "\n";
            //ss << "  " << f << " -> \"" << kv.first << "\"\n";
            (void)f;
        }

        if (rule.antecedent_facts.size() == 0) {
            ss << "  \"" << kv.first << "\"\n";
        } else for (const auto &f: rule.antecedent_facts) {
            ss << "  \"" << kv.first << "\" -> " << f << "\n";
            //ss << "  " << f << " -> \"" << kv.first << "\"\n";
            (void)f;
        }
    }

    ss << "}\n";
    return ss.str();
}


void Digraph::addFact(const Fact &fact) {
    auto it = facts.find(fact.id);
    if (it == facts.end()) {
        (void)facts.insert({fact.id, fact});
        return;
    }

    Fact &existing = it->second;

    std::set<Fact::State> s = {existing.state, fact.state};

    if (s.size() == 1) {
        ; // used to filter for following cases
    } else if (s.contains(Fact::State::True)
            && s.contains(Fact::State::False)) {
        throw std::runtime_error("Conflicting facts");
    } else if (s.contains(Fact::State::True)) {
        existing.state = Fact::State::True;
    } else {
        existing.state = Fact::State::False;
    }

    existing.antecedent_rules = (
        existing.antecedent_rules + fact.antecedent_rules);
    existing.consequent_rules = (
        existing.consequent_rules + fact.consequent_rules);
    return;
}


void Digraph::addRule(const Rule &rule) {
    auto it = rules.find(rule.id);
    if (it != rules.end()) {
        throw std::runtime_error("Duplicate rule");
    }

    if (!rule.expr.isValidRule()) {
        throw std::runtime_error("Invalid rule");
    }


    auto g = rule.expr.getValues();

    if(g.lhs && g.rhs) {
        Rule newRule = rule;

        auto rhsLabels = g.rhs->getAllFacts();
        for (auto const &fl : rhsLabels) {
            auto fact = Fact(fl, Fact::State::Undetermined);
            fact.consequent_rules.push_back(rule.id);
            newRule.consequent_facts.push_back(fact.id);
            addFact(fact);
        }

        auto lhsLabels = g.lhs->getAllFacts();
        for (auto const &fl : lhsLabels) {
            auto fact = Fact(fl, Fact::State::Undetermined);
            fact.antecedent_rules.push_back(rule.id);
            newRule.antecedent_facts.push_back(fact.id);
            addFact(fact);
        }
        rules.insert({newRule.id, newRule});
    } else {
        throw std::runtime_error("Illegal state, rules must have lhs, rhs");
    }

    // TODO : handel if-and-only-if which adds a second rule with terms flipped
    return;
}

void Digraph::setExprVarsToTrue(const Expr &expr) {

    if (auto v = std::get_if<Var>(&expr)) {
        auto it = facts.find(v->value());
        if (it == facts.end()) {
            throw std::runtime_error("Fact not found, but must exist");
        }
        Fact &fact(it->second);
        if (fact.state == Fact::State::False) {
            throw std::runtime_error("Cannont set fact " + 
                    fact.toString() + "  to true, it's already false");
        }
        fact.state = Fact::State::True;
    }

    // if expr is a Var, find this var and set it to true, throw on conflict
    
    // if expr is And, recursivly call and set children to true
    
    // else throw not handled yet
}

Fact::State Digraph::solveForFact(const char fact_id) {
    auto f = facts.find(fact_id);

    if (f == facts.end()){
        throw std::runtime_error("Fact not found, impossible to solve!");
    }

    Fact &fact(f->second);

    for (const auto &r : fact.consequent_rules) {
        std::cout << "solveForFact : solving " << r << std::endl;
        solveRule(r);
    }

    return fact.state;
}


Fact::State Digraph::solveRule(const std::string &rule_id) {
    auto r = rules.find(rule_id);
    if (r == rules.end()) {
        throw std::runtime_error("Rule not found!");
    }

    Rule &rule(r->second);
    auto res = solveExpr(rule.expr);
    std::cout << "solveRule : result " << res << std::endl;
    return res;
}

Fact::State Digraph::solveExpr(const Expr &expr) {
    struct Solver {
        Digraph &digraph;

        Fact::State operator()(const Empty &)
            {return Fact::State::Undetermined;}

        Fact::State operator()(const Var &v)
        {
            auto it = digraph.facts.find(v.value());
            if (it == digraph.facts.end()){
                throw std::runtime_error("Fact not found in digraph!");
            }

            std::cout << "IN Var " << v << " "  << it->second.state << std::endl;

            if (it->second.state == Fact::State::Undetermined) {
                // if state is undefined, it needs to be solved for!
                // leave it for now, afraid of infinite recursion
                // Fact::State res = std::visit(*this, digraph.rules[0].expr);
                digraph.solveForFact(it->second.id);
            }
            return it->second.state;
        }

        Fact::State operator()(const Not &n)
        {
            std::cout << "IN Not" << n << std::endl;
            return visit(*this, n.child()) == Fact::State::True
                ? Fact::State::False
                : visit(*this, n.child()) == Fact::State::False
                    ? Fact::State::True
                    : Fact::State::Undetermined;
        }

        Fact::State operator()(const And &n)
        {
            std::cout << "IN And " << n << std::endl;
            Fact::State lhs = visit(*this, n.lhs());
            Fact::State rhs = visit(*this, n.rhs());
            if (lhs == Fact::State::False || rhs == Fact::State::False) {
                return Fact::State::False;
            }
            if (lhs == Fact::State::True && rhs == Fact::State::True) {
                return Fact::State::True;
            }
            return Fact::State::Undetermined; 
        }

        Fact::State operator()(const Or &n)
        {
            std::cout << "IN Or " << n << std::endl;
            Fact::State lhs = visit(*this, n.lhs());
            Fact::State rhs = visit(*this, n.rhs());
            if (lhs == Fact::State::True || rhs == Fact::State::True) {
                return Fact::State::True;
            }
            if (lhs == Fact::State::False && rhs == Fact::State::False) {
                return Fact::State::False;
            }
            return Fact::State::Undetermined;
        }

        Fact::State operator()(const Xor &n)
        {
            std::cout << "IN Xor " << n << std::endl;
            Fact::State lhs = visit(*this, n.lhs());
            Fact::State rhs = visit(*this, n.rhs());
            if (lhs == Fact::State::Undetermined || rhs == Fact::State::Undetermined) {
                return Fact::State::Undetermined;
            }
            if (lhs != rhs) {
                return Fact::State::True;
            }
            return Fact::State::False;
        }

        // (A ⇒ B) ⇔ (¬A ∨ B)
        Fact::State operator()(const Imply &n)
        {
            std::cout << "IN Imply " << n << std::endl;
            Expr lhs_real = n.lhs();
            Expr rhs_real = n.rhs();

            Fact::State lhs_result = visit(*this, n.lhs());
            // visiting rhs will lead to infinate recursion
            // Fact::State rhs_result = visit(*this, n.rhs());
            //(void)rhs_result;
            if (lhs_result == Fact::State::Undetermined) {
                return Fact::State::Undetermined;
            }
            if (lhs_result == Fact::State::True) {
                std::cout << "Setting " << rhs_real <<" to true\n";
                digraph.setExprVarsToTrue(rhs_real);
            }
            std::cout << "Solving Imply" << std::endl;

            return visit(*this, Expr(Or(Not(lhs_real), rhs_real)));
        }

        // (A ⇔ B) ⇔ ((A ⇒ B) ∧ (B ⇒ A))
        Fact::State operator()(const Iff &n)
        {
            std::cout << "IN Iff " << n << std::endl;
            Expr lhs_real = n.lhs();
            Expr rhs_real = n.rhs();

            Fact::State lhs_result = visit(*this, n.lhs());
            Fact::State rhs_result = visit(*this, n.rhs());
            if (lhs_result == Fact::State::Undetermined || rhs_result == Fact::State::Undetermined) {
                return Fact::State::Undetermined;
            }
            lhs_real = Imply(lhs_real, rhs_real);
            rhs_real = Imply(rhs_real, lhs_real);
            Expr both = And(lhs_real, rhs_real);
            return visit(*this, both);
        }
    };
    return std::visit(Solver{*this}, expr);
}


Digraph makeDigraph(
        const std::vector<Fact> &facts,
        const std::vector<Rule> &rules) {
    Digraph g;

    for (const auto &f : facts) {
        g.addFact(f);
    }

    for (const auto &r : rules) {
        g.addRule(r);
    }

    return g;
}

