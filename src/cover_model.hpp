#ifndef COVER_MODEL_HPP
#define COVER_MODEL_HPP

// The exact cover instance: rho-orbits as columns, admissible walks as rows
// (section 3.3).

#include "conditions.hpp"
#include "exact_cover.hpp"
#include "junctions.hpp"

#include <cstddef>
#include <map>
#include <vector>

namespace cover_model {

using Columns = std::map<dna::Word, std::size_t>;

Columns orbitColumns();
// Candidates that can appear in some admissible set (see Remark 3.4.1).
conditions::SymbolList usableCandidates();

conditions::SymbolList allowedUnder(const conditions::SymbolList& candidates,
                                    const std::vector<junctions::Split>& splits,
                                    unsigned choice);

exact_cover::Solver buildSolver(const conditions::SymbolList& symbols,
                                const Columns& columns);

}  // namespace cover_model

#endif
