#ifndef CONFLICTS_HPP
#define CONFLICTS_HPP

// Largest subset of a symbol list that satisfies conditions (A), (B1) and (B2)
// (section 3.8).

#include "conditions.hpp"
#include "dna.hpp"

#include <cstddef>
#include <vector>

namespace conflicts {

using Graph = std::vector<std::vector<bool>>;  // adjacency matrix

// The edges of u and v share a rho-orbit, or uv or vu contains a recognition site.
bool inConflict(const dna::Word& u, const dna::Word& v);

struct ConflictGraph {
    conditions::SymbolList usable;    // symbols that may appear in some admissible set
    conditions::SymbolList rejected;  // symbols that may not
    Graph adjacent;                   // indexed like usable

    std::size_t conflictCount() const;
};

ConflictGraph build(const conditions::SymbolList& symbols);

// Exact search, exponential in the worst case; meant for lists of a few dozen symbols.
std::vector<std::size_t> largestIndependentSet(const Graph& adjacent);

conditions::SymbolList largestAdmissibleSubset(const ConflictGraph& graph);

}  // namespace conflicts

#endif
