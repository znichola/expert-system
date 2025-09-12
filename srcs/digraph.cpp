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

