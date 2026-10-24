#ifndef CONDITIONS_HPP
#define CONDITIONS_HPP

// Admissibility conditions from chapter 2 of the thesis.

#include "dna.hpp"

#include <string>
#include <vector>

namespace conditions {

using SymbolList = std::vector<dna::Word>;

struct StrandPair {
    dna::Word strand1;
    dna::Word strand2;
};

struct CheckResult {
    std::string name;
    bool passed = false;
    std::string detail;
};

// Whether the symbol can appear in some admissible set: it is a candidate
// (Definition 3.4.2) and uu contains no recognition site (Remark 3.5.1).
bool mayAppearInAdmissibleSet(const dna::Word& symbol);

CheckResult symbolsWellFormed(const SymbolList& symbols);
CheckResult symbolsDistinct(const SymbolList& symbols);
CheckResult walksConsistent(const SymbolList& symbols);
CheckResult edgesDistinct(const SymbolList& symbols);

// condition (A)
CheckResult noSelfComplementaryEdges(const SymbolList& symbols);
CheckResult noSharedOrbit(const SymbolList& symbols);
CheckResult transversalComplete(const SymbolList& symbols);

// conditions (B1) and (B2)
CheckResult noSiteWithinSymbols(const SymbolList& symbols);
CheckResult noSiteAcrossJunctions(const SymbolList& symbols);

CheckResult partitionBalanced(const SymbolList& first,
                              const SymbolList& second,
                              const SymbolList& third);
CheckResult strandsComplementary(const std::vector<StrandPair>& rows);

std::vector<CheckResult> verifyAll(const SymbolList& symbols);
std::vector<SymbolList> layersOf(const SymbolList& symbols);

}  // namespace conditions

#endif
