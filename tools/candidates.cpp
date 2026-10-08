// Counts the admissible walks of length 3, i.e. the rows of the cover matrix.
#include "conditions.hpp"
#include "dna.hpp"

#include <iostream>
#include <set>
#include <vector>

namespace {

using dna::Word;

}  // namespace

int main() {
    const std::vector<Word> symbols = dna::allWords(dna::SYMBOL_LENGTH);

    std::size_t total = symbols.size();
    std::size_t freeOfSites = 0;     // (B1)
    std::size_t selfJunctionOk = 0;  // (B2) for the pair (u,u)
    std::size_t candidates = 0;
    std::set<Word> coveredOrbits;

    for (const Word& symbol : symbols) {
        if (!dna::containsRecognitionSite(symbol)) ++freeOfSites;
        if (!dna::containsRecognitionSite(symbol + symbol)) ++selfJunctionOk;
        if (!conditions::mayAppearInAdmissibleSet(symbol)) continue;

        ++candidates;
        for (const Word& edge : dna::symbolEdges(symbol))
            coveredOrbits.insert(dna::orbitRepresentative(edge));
    }

    std::cout << "Words of length 6 (all walks of length 3): " << total << "\n"
              << "  free of a site inside          (B1): " << freeOfSites << "\n"
              << "  free of a site in uu       (B2, u=v): " << selfJunctionOk << "\n"
              << "  usable in some admissible set      : " << candidates << "\n"
              << "  orbits reachable by a candidate    : " << coveredOrbits.size()
              << " of " << dna::countOrbits(dna::EDGE_LENGTH) << "\n";
    return 0;
}
