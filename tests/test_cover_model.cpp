// The exact cover instance (section 3.3), checked against the counts in the thesis.
#include "conditions.hpp"
#include "cover_model.hpp"
#include "dna.hpp"
#include "testing.hpp"

#include <set>

int main() {
    Suite suite("cover_model");

    const cover_model::Columns columns = cover_model::orbitColumns();
    std::set<std::size_t> indices;
    for (const auto& [orbit, index] : columns) indices.insert(index);
    suite.expect("120 columns numbered 0..119",
                 columns.size() == 120 && indices.size() == 120 && *indices.rbegin() == 119);

    // Remark 3.3.1 and Remark 3.4.1: 4096 words -> 3336 -> 3324 candidates -> 3306.
    std::size_t distinctOrbits = 0, withoutSite = 0;
    for (const dna::Word& word : dna::allWords(dna::SYMBOL_LENGTH)) {
        std::set<dna::Word> orbits;
        bool selfComplementary = false;
        for (const dna::Word& edge : dna::symbolEdges(word)) {
            selfComplementary = selfComplementary || dna::isSelfComplementary(edge);
            orbits.insert(dna::orbitRepresentative(edge));
        }
        if (selfComplementary || orbits.size() != 3) continue;
        ++distinctOrbits;
        if (!dna::containsRecognitionSite(word)) ++withoutSite;
    }
    suite.expect("3336 words have three edges in three orbits", distinctOrbits == 3336);
    suite.expect("3324 of them are candidates", withoutSite == 3324);

    const conditions::SymbolList candidates = cover_model::usableCandidates();
    suite.expect("3306 remain once uu is required to be free of a site",
                 candidates.size() == 3306);

    const std::vector<junctions::Split> splits = junctions::allSplits();
    bool filterAgrees = true;
    for (const dna::Word& symbol : cover_model::allowedUnder(candidates, splits, 0))
        filterAgrees = filterAgrees && junctions::allowed(symbol, splits, 0);
    suite.expect("allowedUnder keeps only symbols allowed by the choice", filterAgrees);

    return suite.finish();
}
