#include <cstddef>
#include <vector>

#include "expert-system.hpp"

struct SolverResult {
    std::vector<size_t> reasoning;
    std::vector<Fact> facts;
};

