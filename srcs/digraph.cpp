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

    // TODO change to accept complicated rules in conclusion
    if (!rule.expr.isValidRule()) {
        throw std::runtime_error("Invalid rule: " + rule.toString());
    }

    // Check if this is an Iff expression
    if (auto iff = std::get_if<Iff>(&rule.expr)) {
        // For A <=> B, create two rules: A => B and B => A
        Expr forward_rule = Imply(iff->lhs(), iff->rhs());
        Expr backward_rule = Imply(iff->rhs(), iff->lhs());
        
        // Create and add the forward rule (A => B)
        Rule forward_r(forward_rule, rule.line_number, rule.comment + " (forward)");
        addRule(forward_r);
        
        // Create and add the backward rule (B => A)  
        Rule backward_r(backward_rule, rule.line_number, rule.comment + " (backward)");
        addRule(backward_r);
        
        return;
    }

    // Handle normal rules (non-Iff)
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

    return;
}

void Digraph::setExprVarsToState(const Expr &expr, const Fact::State state) {
    if (auto v = std::get_if<Var>(&expr)) {
        auto it = facts.find(v->value());
        if (it == facts.end()) {
            throw std::runtime_error("Fact not found, but must exist");
        }
        Fact &fact(it->second);

        if (fact.state == Fact::State::Undetermined) {
            fact.state = state;
        } else if (state != fact.state) {
            std::stringstream ss;
            ss << "Contradiciton: Can't set fact " << fact << " to "
               << state << " it's already " << fact.state;
            throw std::runtime_error(ss.str());
        } 
        
        // TODO should we allow setting to Undetermined? 
        // else {
        //     std::cout
        //     << "No point in setting expr to Undetermined, it is by default\n";
        // }
        return ;
    }
    else if (auto n = std::get_if<Not>(&expr)) {
        setExprVarsToState(n->child(), state == Fact::State::True
                ? Fact::State::False
                : state == Fact::State::False
                    ? Fact::State::True
                    : Fact::State::Undetermined);
        return ;
    }
    else if (auto and_expr = std::get_if<And>(&expr)) {
        // And handling
        if (state == Fact::State::True) {
            // A + B = True means both A and B must be True
            setExprVarsToState(and_expr->lhs(), Fact::State::True);
            setExprVarsToState(and_expr->rhs(), Fact::State::True);
        } else if (state == Fact::State::False) {
            // A + B = False: we can't determine individual values
            setExprVarsToState(and_expr->lhs(), Fact::State::Undetermined);
            setExprVarsToState(and_expr->rhs(), Fact::State::Undetermined);
        }
        return;
    }
    else if (auto or_expr = std::get_if<Or>(&expr)) {
        // Or handling  
        if (state == Fact::State::True) {
            // A | B = True: we can't determine which one is true
            // Both could be true, or just one - ambiguous
            // set both to undetermined
            setExprVarsToState(or_expr->lhs(), Fact::State::Undetermined);
            setExprVarsToState(or_expr->rhs(), Fact::State::Undetermined);
        } else if (state == Fact::State::False) {
            // A | B = False means both A and B must be False
            setExprVarsToState(or_expr->lhs(), Fact::State::False);
            setExprVarsToState(or_expr->rhs(), Fact::State::False);
        }
        return;
    }
    else if (auto xor_expr = std::get_if<Xor>(&expr)) {
        // Xor handling
        if (state == Fact::State::True) {
            // A ^ B = True: exactly one is true, but we don't know which
            // Can't determine individual values without more info
            // set both to undetermined
            setExprVarsToState(xor_expr->lhs(), Fact::State::Undetermined);
            setExprVarsToState(xor_expr->rhs(), Fact::State::Undetermined);
        } else if (state == Fact::State::False) {
            // A ^ B = False: either both true or both false
            // Can't determine individual values without more info
            setExprVarsToState(xor_expr->lhs(), Fact::State::Undetermined);
            setExprVarsToState(xor_expr->rhs(), Fact::State::Undetermined);
        }
        return;
    }
    throw std::runtime_error("Set facts not handled for this expr "
            + expr.toString());

    // if expr is a Var, find this var and set it to true, throw on conflict

    // if expr is And, recursivly call and set children to true

    // else throw not handled yet
}

Fact::State Digraph::solveForFact(const char fact_id, bool isExplain) {
    auto f = facts.find(fact_id);

    if (f == facts.end()){
        throw std::runtime_error("Fact not found, impossible to solve!");
    }

    Fact &fact(f->second);

    // Check for cycle
    if (solving_stack.find(fact_id) != solving_stack.end()) {
        if (isExplain) {
            std::cout << "Cycle detected for fact " << fact_id << ", setting to False" << std::endl;
        }
        fact.state = Fact::State::False;
        return fact.state;
    }

    // Add to solving stack
    solving_stack.insert(fact_id);

    for (const auto &r : fact.consequent_rules) {
        if (isExplain) {
            std::cout << "solveForFact " << fact_id << ": solving " << r << std::endl;
        }
        solveRule(r, isExplain);
    }

    // Remove from solving stack
    solving_stack.erase(fact_id);

    return fact.state;
}


Fact::State Digraph::solveRule(const std::string &rule_id, bool isExplain) {
    auto r = rules.find(rule_id);
    if (r == rules.end()) {
        throw std::runtime_error("Rule not found!");
    }

    Rule &rule(r->second);
    auto res = solveExpr(rule.expr, isExplain);
    if (isExplain) {
        std::cout << "solveRule " << rule_id << ": result " << res << std::endl;
    }
    return res;
}

Fact::State Digraph::solveExpr(const Expr &expr, bool isExplain) {
    struct Solver {
        Digraph &digraph;
        bool isExplain;

        Fact::State operator()(const Empty &)
            {return Fact::State::Undetermined;}

        Fact::State operator()(const Var &v)
        {
            auto it = digraph.facts.find(v.value());
            if (it == digraph.facts.end()){
                throw std::runtime_error("Fact not found in digraph!");
            }

            if (isExplain) {
                std::cout << "IN Var " << v << " "  << it->second.state << std::endl;
            }

            if (it->second.state == Fact::State::Undetermined) {
                digraph.solveForFact(it->second.id, isExplain);
            }
            return it->second.state;
        }

        Fact::State operator()(const Not &n)
        {
            if (isExplain) {
                std::cout << "IN Not" << n << std::endl;
            }
            return visit(*this, n.child()) == Fact::State::True
                ? Fact::State::False
                : visit(*this, n.child()) == Fact::State::False
                    ? Fact::State::True
                    : Fact::State::Undetermined;
        }

        Fact::State operator()(const And &n)
        {
            if (isExplain) {
                std::cout << "IN And " << n << std::endl;
            }
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
            if (isExplain) {
                std::cout << "IN Or " << n << std::endl;
            }
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
            if (isExplain) {
                std::cout << "IN Xor " << n << std::endl;
            }
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
            if (isExplain) {
                std::cout << "IN Imply " << n << std::endl;
            }
            
            Expr lhs_real = n.lhs();
            Expr rhs_real = n.rhs();
            Fact::State lhs_result = visit(*this, lhs_real);
            
            if (isExplain) {
                std::cout << "Imply: LHS (" << lhs_real << ") = " << lhs_result << std::endl;
            }
            
            // If antecedent is True, consequent must be True
            if (lhs_result == Fact::State::True) {
                if (isExplain) {
                    std::cout << "Setting " << rhs_real << " to True (antecedent is True)" << std::endl;
                }
                digraph.setExprVarsToState(rhs_real, Fact::State::True);
                return Fact::State::True;
            }
            
            // If antecedent is False, implication is True (vacuously true)
            if (lhs_result == Fact::State::False) {
                return Fact::State::True;
            }
            
            // If antecedent is Undetermined, we can't conclude anything
            return Fact::State::Undetermined;
        }

        // (A ⇔ B) ⇔ ((A ⇒ B) ∧ (B ⇒ A))
        Fact::State operator()(const Iff &n)
        {
            if (isExplain) {
            std::cout << "IN Iff " << n << std::endl;
            }
        
            // Get the actual operands from the Iff expression
            Expr lhs_real = n.lhs();
            Expr rhs_real = n.rhs();
            
            // Create the two implications: (A ⇒ B) and (B ⇒ A)
            Expr forward_imply = Imply(lhs_real, rhs_real);
            Expr backward_imply = Imply(rhs_real, lhs_real);
            
            // Combine them with AND: ((A ⇒ B) ∧ (B ⇒ A))
            Expr both = And(forward_imply, backward_imply);
            
                return visit(*this, both);
        }
    };
    return std::visit(Solver{*this, isExplain}, expr);
}


Digraph makeDigraph(
        const std::vector<Fact> &facts,
        const std::vector<Rule> &rules
    ) {
    Digraph g;

    for (const auto &f : facts) {
        g.addFact(f);
    }

    for (const auto &r : rules) {
        g.addRule(r);
    }

    return g;
}

