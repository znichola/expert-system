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

    return false;
}


bool Digraph::addRule(const Rule &rule) {
    auto res = rules.insert({rule.id, rule});
    std::cout << "Inserted: " << res.second << std::endl;


    // Get facts from lhs
    //   add the currnt rule to the links for these facts

    // Get facts form rhs
    //   add these facts to fact list, and this rules deductions


    return false;
}


