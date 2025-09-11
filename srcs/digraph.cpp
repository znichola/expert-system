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
            // continue statement here
        }

        for (const auto &r: fact.consequent_rules) {
            ss << "  " << kv.first << " -> " << r << "\n";
        }

    }

    for (const auto &kv : rules) {
        auto rule = kv.second;

        if (rule.antecedent_facts.size() == 0) {
            ss << "  \"" << kv.first << "\"\n";
        }

        for (const auto &f: rule.antecedent_facts) {
            ss << "  " << kv.first << " -> " << f << "\n";
        }
    }

    ss << "}\n";
    return ss.str();
}


bool Digraph::addFact(const Fact &fact) {
    auto it = facts.find(fact.id);
    if (it == facts.end()) {
        (void)facts.insert({fact.id, fact});
        return true;
    }

    Fact &existing = it->second;

    if (fact.state == existing.state) {
        existing.antecedent_rules 
        return true;
    }

    if (fact.state == existing.state) {
        return true;
    }
    //std::cout << "Inserted: " << res.second << std::endl;

    // Ensure the fact is merged with existing facts
    // If state is equal, add it
    // propmote undetermined state to true or false
    // 
    // If True / False conficlt, throw error
    //
    // if True but new is unetermined, then still add it

    return false;
}


bool Digraph::addRule(const Rule &rule) {

    // Rule validation, before inserting

    auto res = rules.insert({rule.id, rule});
    //std::cout << "Inserted: " << res.second << std::endl;
    (void)res;

    // With res check rule duplication

    auto id = rule.id;

    auto factLabels = rule.expr.getAllFacts();
    for (auto const &fl : factLabels) {
        addFact(Fact(fl, Fact::State::Undetermined));
    }

    // Get only facts from lhs
    //   add to the consquent_facts

    // Get only facts form rhs
    //   add to the antecedent_facts

    // If duplicate rule throw error, maybe? could also just ignore the dupe

    return false;
}


Fact::State Digraph::solveFor(const Query &query) {
    auto f = facts.find(query.label);

    if (f == facts.end()){
        std::cout << "Fact not found, not possible!" << std::endl;
        return Fact::State::Undetermined;
    }
    Fact fact(f->second);

    for (const auto &r : fact.consequent_rules) {
        (void)r;
        // r : solve this rule, if possible, see if this solves the fact
    }

    return Fact::State::Undetermined;
}




