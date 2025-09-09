#include "expert-system.hpp"

#include <algorithm>
#include <iostream>


Foo solve(
        const std::vector<Rule> &rules, 
        const std::vector<Fact> &facts,
        const Query &query) {

    auto is_an_existing_fact = facts.end() != std::ranges::find_if(facts,
            [&query](auto const &f) { return f.label == query.label; });
    if (is_an_existing_fact) {
        std::cout << "Query is solved";
        return {}; // this query is solved, plus the new facts list
    }



    return {};
}
