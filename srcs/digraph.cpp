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


bool Digraph::addFact(const Fact &fact) {
    auto res = facts.insert({fact.id, fact});
    std::cout << "Inserted: " << res.second << std::endl;

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
    std::cout << "Inserted: " << res.second << std::endl;

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


