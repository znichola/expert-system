#include <set>
#include <functional>

#include "expert-system.hpp"
#include "vector_helper.hpp"

Digraph::SolveRes Digraph::solveEverythingNoThrow(const std::vector<Query> &queries) {
    std::ostringstream conclusion;
    std::ostringstream explanation;
    bool isError = false;
    std::map<char, Expr> compiledExpressions;
    for (const auto &query : queries) {
        auto e = compileExprForFact(query.label);
        compiledExpressions.insert({query.label, e});
    }
    for (const auto &query : queries) {
        try {
            auto res = solveForFact(query.label);
            auto expr = compiledExpressions.at(query.label);
            auto table = boolMapEvaluate(expr);
            res = determinFinalState(res, table, query.label);
            conclusion << query.label << " is " << res << std::endl;
            explanation << query.label << " ⇔ " << std::visit(PrinterFormalLogic{}, expr) << std::endl;
            explanation << table << std::endl;
        } catch (const std::exception &e) {
            conclusion << query << " Error: " << e.what() << std::endl;
            isError = true;
        }
    }
    if (isExplain) {
        explanation << "OPERATIONS\n" << this->explanation.str();
    }
    return {conclusion.str(), explanation.str(), isError};
}

Fact::State Digraph::determinFinalState(Fact::State solverRes, const VarBoolMap &boolMap, char fact_id) {
  
    if (!boolMap.contains(fact_id)) {
        if (isExplain)
            explanation << fact_id 
            << ": Truth table is empty — no combination of variables satisfies all rules; there is a contradiction in the rule set.\n";
        return solverRes;
    }

    const auto &values = boolMap.at(fact_id);

    if (values.empty()) return solverRes;

    // Determine result from the truth table
    bool all_true = std::all_of(values.begin(), values.end(), [](bool v){ return v; });
    bool all_false = std::all_of(values.begin(), values.end(), [](bool v){ return !v; });

    Fact::State boolMapResult = all_true ? Fact::State::True : all_false ? Fact::State::False : Fact::State::Undetermined;

    // Closed-world assumption: check if this fact is the only one differing
    if (boolMapResult == Fact::State::Undetermined) {
        bool differsOnlyHere = true;

        for (const auto &[other_id, other_values] : boolMap) {
            if (other_id == fact_id) continue;
            if (other_values.size() != values.size()) continue; // inconsistent data, skip

            for (size_t i = 0; i < values.size(); ++i) {
                // if any other fact changes when this one changes, not closed-world false
                if (other_values[i] != other_values.front()) {
                    differsOnlyHere = false;
                    break;
                }
            }
            if (!differsOnlyHere) break;
        }

        if (differsOnlyHere) {
            if (isExplain)
                explanation << fact_id << ": Determined false by closed-world assumption (only it changes)\n";
            boolMapResult = Fact::State::False;
        }
    }

    if (solverRes == boolMapResult) return solverRes;

    if (solverRes == Fact::State::Undetermined) {
        if (isExplain)
            explanation << fact_id << ": Deferring to truth table evaluation\n";
        return boolMapResult;
    }

    if (isExplain)
        explanation << fact_id << ": Oops.. solver and boolean table disagree on fact, deferring to solver\n";
    return solverRes;
}



void Digraph::applyWorldAssumption(bool open) {
    if (open) {
        if (isExplain) {
            explanation << "Applying Open World Assumption: Facts are Undetermined by default" << std::endl;
        }
        return ;
    }
    for (auto &[fact_id, fact]: facts) {
        if (fact.state == Fact::State::Undetermined && fact.consequent_rules.empty()) {
            if (isExplain) {
                explanation << "Applying Closed World Assumption: " << fact_id << " = False (no rules can prove it)" << std::endl;
            }
            fact.state = Fact::State::False;
        }
    }
}


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

bool Digraph::isFactInAmbiguousConclusion(char fact_id) const {
    auto fact_it = facts.find(fact_id);
    if (fact_it == facts.end()) return false;
    
    // Check if this fact appears in any rule conclusion that creates ambiguity
    for (const auto &rule_id : fact_it->second.consequent_rules) {
        auto rule_it = rules.find(rule_id);
        if (rule_it != rules.end()) {
            // Check if the rule's conclusion is an Or, Xor, or other ambiguous expression
            // that when set to True doesn't uniquely determine this fact
            auto values = rule_it->second.expr.getValues();
            if (values.rhs) {
                if (std::holds_alternative<Or>(*(values.rhs)) ||
                    std::holds_alternative<Xor>(*(values.rhs))) {
                    return true; // This fact is in an ambiguous conclusion
                }
            }
        }
    }
    return false;
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

    if (auto iff = std::get_if<Iff>(&rule.expr)) {
        auto g = rule.expr.getValues();
        if (g.lhs && g.rhs) {
            Rule newRule = rule;

            auto rhsLabels = g.rhs->getAllFacts();
            for (auto const &fl : rhsLabels) {
                auto fact = Fact(fl, Fact::State::Undetermined);
                fact.antecedent_rules.push_back(rule.id);
                newRule.antecedent_facts.push_back(fact.id);
                fact.consequent_rules.push_back(rule.id);
                newRule.consequent_facts.push_back(fact.id);
                addFact(fact);
            }

            auto lhsLabels = g.lhs->getAllFacts();
            for (auto const &fl : lhsLabels) {
                auto fact = Fact(fl, Fact::State::Undetermined);
                fact.antecedent_rules.push_back(rule.id);
                newRule.antecedent_facts.push_back(fact.id);
                fact.consequent_rules.push_back(rule.id);
                newRule.consequent_facts.push_back(fact.id);
                addFact(fact);
            }
            rules.insert({newRule.id, newRule});
        }
        else
        {
            throw std::runtime_error("Illegal state, rules must have lhs, rhs");
        }
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
        } else if (fact.state == state || state == Fact::State::Undetermined) { // if same state or determined facts to undetermined
            // Same state, no problem
            return;
        } else {
            // Real contradiction
            std::stringstream ss;
            ss << "Contradiction: Can't set fact " << fact << " to "
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
            // A | B = True: At least one must be true, so if one is false we know the other is true
            Fact::State lhs_state = solveExpr(or_expr->lhs());
            Fact::State rhs_state = solveExpr(or_expr->rhs());
    
                 if (lhs_state == Fact::State::False) { setExprVarsToState(or_expr->rhs(), Fact::State::True); }
            else if (rhs_state == Fact::State::False) { setExprVarsToState(or_expr->lhs(), Fact::State::True); }
            else {
                setExprVarsToState(or_expr->lhs(), Fact::State::Undetermined);
                setExprVarsToState(or_expr->rhs(), Fact::State::Undetermined);
            }
        } else if (state == Fact::State::False) {
            // A | B = False: Both A and B must be False
            setExprVarsToState(or_expr->lhs(), Fact::State::False);
            setExprVarsToState(or_expr->rhs(), Fact::State::False);
        }
        return;
    }
    else if (auto xor_expr = std::get_if<Xor>(&expr)) {
        if (state == Fact::State::Undetermined) {
            // Do nothing, nothing can be learnt from this, TODO: if we add fourth uninitialised state this changes
            return ;
        }
        Fact::State lhs_state = solveExpr(xor_expr->lhs());
        Fact::State rhs_state = solveExpr(xor_expr->rhs());

        if (lhs_state == Fact::State::Undetermined && lhs_state == Fact::State::Undetermined) {
            // Do nothing, nothing can be learnt from this, TODO: if we add fourth uninitialised state this changes
            return ;
        }
        if (state == Fact::State::True) {
            // A ^ B = True: exactly one must be true
                 if (lhs_state == Fact::State::True) { setExprVarsToState(xor_expr->rhs(), Fact::State::False); }
            else if (rhs_state == Fact::State::True) { setExprVarsToState(xor_expr->lhs(), Fact::State::False); }

            else if (lhs_state == Fact::State::False) { setExprVarsToState(xor_expr->rhs(), Fact::State::True); }
            else if (rhs_state == Fact::State::False) { setExprVarsToState(xor_expr->lhs(), Fact::State::True); }

            // Undetermined cases are handled above if you think about it

        } else if (state == Fact::State::False) {
            // A ^ B = False: both true or both false
                 if (lhs_state == Fact::State::True) { setExprVarsToState(xor_expr->rhs(), Fact::State::True); }
            else if (rhs_state == Fact::State::True) { setExprVarsToState(xor_expr->lhs(), Fact::State::True); }

            else if (lhs_state == Fact::State::False) { setExprVarsToState(xor_expr->rhs(), Fact::State::False); }
            else if (rhs_state == Fact::State::False) { setExprVarsToState(xor_expr->lhs(), Fact::State::False); }
        }
        return;
    }
    throw std::runtime_error("Set facts not handled for this expr "
            + expr.toString());

    // if expr is a Var, find this var and set it to true, throw on conflict

    // if expr is And, recursivly call and set children to true

    // else throw not handled yet
}

Fact::State Digraph::solveForFact(const char fact_id) {
    auto f = facts.find(fact_id);
    if (f == facts.end()){
        if (isClosedWorldAssumption) {
            if (isExplain) {
                explanation << "Applying Closed World Assumption: " << fact_id << " = False" << std::endl;
            }
            return Fact::State::False;
        } else {
            if (isExplain) {
                explanation << "Applying Open World Assumption: " << fact_id << " = Undetermined" << std::endl;
            }
            return Fact::State::Undetermined;
        }
    }

    Fact &fact(f->second);

    // Check for cycle
    if (solving_stack.find(fact_id) != solving_stack.end()) {
        if (isExplain) {
            explanation << "Cycle detected for fact " << fact_id << ", deferring to other rules" << std::endl;
        }
        // Don't set to False immediately - return undetermined and let other rules try
        return Fact::State::Undetermined;
    }

    // Add to solving stack
    solving_stack.insert(fact_id);

    for (const auto &r : fact.consequent_rules) {
        if (isExplain) {
            explanation << "solveForFact " << fact_id << ": solving " << r << std::endl;
        }
        solveRule(r); // TODO do we need a check if the rule returns true, (it should always be true, I think?)
    }

    // Remove from solving stack
    solving_stack.erase(fact_id);

    return fact.state;
}

// Helper function to count determined antecedents in a rule
int Digraph::countDeterminedAntecedents(const std::string& rule_id) {
    auto rule_it = rules.find(rule_id);
    if (rule_it == rules.end()) return 0;

    auto antecedent_facts = rule_it->second.antecedent_facts;
    int determined_count = 0;
    
    for (char fact_id : antecedent_facts) {
        auto fact_it = facts.find(fact_id);
        if (fact_it != facts.end() && fact_it->second.state != Fact::State::Undetermined) {
            determined_count++;
        }
    }
    
    return determined_count;
}

Fact::State Digraph::solveRule(const std::string &rule_id) {
    auto r = rules.find(rule_id);
    if (r == rules.end()) {
        throw std::runtime_error("Rule not found!");
    }

    Rule &rule(r->second);
    auto res = solveExpr(rule.expr);
    if (isExplain) {
        explanation << "solveRule " << rule_id << ": result " << res << std::endl;
    }
    // TODO : should their be a check for rules that resolve to false, it should be illigal in this sytem, right?
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

            if (digraph.isExplain) {
                digraph.explanation << "IN Var " << v << " "  << it->second.state << std::endl;
            }

            if (it->second.state == Fact::State::Undetermined) {
                return digraph.solveForFact(it->second.id);
            }
            return it->second.state;
        }

        Fact::State operator()(const Not &n)
        {
            if (digraph.isExplain) {
                digraph.explanation << "IN Not" << n << std::endl;
            }
            return visit(*this, n.child()) == Fact::State::True
                ? Fact::State::False
                : visit(*this, n.child()) == Fact::State::False
                    ? Fact::State::True
                    : Fact::State::Undetermined;
        }

        Fact::State operator()(const And &n)
        {
            if (digraph.isExplain) {
                digraph.explanation << "IN And " << n << std::endl;
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
            if (digraph.isExplain) {
                digraph.explanation << "IN Or " << n << std::endl;
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
            if (digraph.isExplain) {
                digraph.explanation << "IN Xor " << n << std::endl;
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
            if (digraph.isExplain) {
                digraph.explanation << "IN Imply " << n << std::endl;
            }
            
            Expr lhs_real = n.lhs();
            Expr rhs_real = n.rhs();
            Fact::State lhs_result = visit(*this, lhs_real);
            
            if (digraph.isExplain) {
                digraph.explanation << "Imply: LHS (" << lhs_real << ") = " << lhs_result << std::endl;
            }
            
            // If antecedent is True, consequent must be True
            if (lhs_result == Fact::State::True) {
                if (digraph.isExplain) {
                    digraph.explanation << "Setting " << rhs_real << " to True (antecedent is True)" << std::endl;
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
            if (digraph.isExplain) {
                digraph.explanation << "In If and only if " << n << std::endl;
            }

            Fact::State lhs_state = visit(*this, n.lhs());
            Fact::State rhs_state = visit(*this, n.rhs());

            if (rhs_state == lhs_state) {
                if (digraph.isExplain) {
                    digraph.explanation << "In Iff: both sides aready equal " << n << std::endl;
                }
                if (rhs_state == Fact::State::Undetermined) {
                    return Fact::State::Undetermined;
                }
            } else if (lhs_state == Fact::State::Undetermined && rhs_state != Fact::State::Undetermined) {
                digraph.setExprVarsToState(n.lhs(), rhs_state);
            } else if (rhs_state == Fact::State::Undetermined && lhs_state != Fact::State::Undetermined) {
                digraph.setExprVarsToState(n.rhs(), lhs_state);
            } else {
                std::stringstream ss;
                ss << "Contradiction: " << n << " lhs:" << lhs_state << " must equal rhs:" << rhs_state;
                throw std::runtime_error(ss.str()); 
            }
            return Fact::State::True;
        }
    };
    return std::visit(Solver{*this}, expr);
}

Digraph::VarBoolMap Digraph::boolMapEvaluate(const Expr &expr) const {
    // const Fact& fact = facts.at(fact_id);
    // const Expr expr = compileExprForFact(fact_id);
    const std::vector<char> all_facts = expr.getAllFacts();

    // Separate known and undetermined facts
    std::set<char> undetermined_set;
    std::map<char, bool> knownValues;

    for (const auto& f_id : all_facts) {
        const auto& f = facts.at(f_id);
        switch (f.state) {
            case Fact::State::True:
                knownValues[f_id] = true;
                break;
            case Fact::State::False:
                knownValues[f_id] = false;
                break;
            case Fact::State::Undetermined:
                undetermined_set.insert(f_id);
                break;
        }
    }

    // Prepare a VarBoolMap to store results for each fact ID
    VarBoolMap results;

    std::vector<char> undetermined(undetermined_set.begin(), undetermined_set.end());;

    // Generate all combinations of truth assignments for undetermined facts
    const size_t n = undetermined.size();
    const size_t total = (1ULL << n);

    for (size_t mask = 0; mask < total; ++mask) {
        std::map<char, bool> varMap = knownValues;

        // assign bits to undetermined facts
        for (size_t i = 0; i < n; ++i) {
            bool value = (mask >> i) & 1;
            varMap[undetermined[i]] = value;
        }

        bool result = expr.booleanEvaluate(varMap);

        // only add varMap if the result is true, the assumption is the ruleset evaluates to true!
        if (result) {
            for (const auto& [fid, val] : varMap) {
                results[fid].push_back(val);
            }
            results['='].push_back(result);
        }
    }

    return results;
}


Expr Digraph::compileExprForFact(const char fact_id) {
    std::vector<std::string> rules_used_ids;
    std::vector<Expr> rules_used;

    // Recursive lambda that collects all rules related to a fact
    std::function<void(const char)> ruleCollector = [&](const char f_id) {
        const Fact &fact = facts.at(f_id);

        if (fact.state == Fact::State::True || fact.state == Fact::State::False) {
            Expr new_rule = fact.state == Fact::State::True ? Expr(Var(f_id)) : Expr(Not(Var(f_id)));
            if (std::find(rules_used_ids.begin(), rules_used_ids.end(), new_rule.toString()) == rules_used_ids.end()) {
                rules_used_ids.push_back(new_rule.toString());
                rules_used.push_back(new_rule);
            }
            return;
        }

        for (const auto& r_id : fact.consequent_rules) {
            const Rule &rule = rules.at(r_id);
            
            // Skip already processed rules to prevent infinite loops
            if (std::find(rules_used_ids.begin(), rules_used_ids.end(), rule.expr.toString()) != rules_used_ids.end())
                continue;

            rules_used_ids.push_back(rule.expr.toString());
            rules_used.push_back(rule.expr);

            // Recursively gather rules from facts referenced requiered by this rule (antecedent)
            for (const auto& f2_id : rule.antecedent_facts) {
                ruleCollector(f2_id);
            }
        }
    };

    // Run the recursive rule collector
    ruleCollector(fact_id);

    // Compile a mega-expression that ANDs all collected rules
    if (rules_used.empty()) {
        if (isExplain) {
            std::cout << "Empty ruleset when compiling\n";
        }
        return Var(fact_id);
    }

    Expr mega_expr = rules_used.front();
    for (size_t i = 1; i < rules_used.size(); ++i) {
        mega_expr = And(mega_expr, Expr(rules_used[i]));
    }

    if (isExplain) {
        explanation << "Compiled logic expression for " << fact_id 
        << " using " << rules_used.size() << " rules\n";
    }

    return mega_expr;
}


Digraph makeDigraph(
        const std::vector<Fact> &facts,
        const std::vector<Rule> &rules,
        const std::vector<Query> &quries
    ) {
    Digraph g;

    for (const auto &f : facts) {
        g.addFact(f);
    }

    for (const auto &q : quries) {
        g.addFact(Fact(q.label, Fact::State::Undetermined));
    }

    for (const auto &r : rules) {
        g.addRule(r);
    }

    return g;
}


