#include "cover_model.hpp"

namespace cover_model {

Columns orbitColumns() {
    Columns columns;
    for (const dna::Word& word : dna::allWords(dna::EDGE_LENGTH)) {
        if (dna::isSelfComplementary(word)) continue;
        const dna::Word representative = dna::orbitRepresentative(word);
        if (!columns.count(representative))
            columns.emplace(representative, columns.size());
    }
    return columns;
}

conditions::SymbolList usableCandidates() {
    conditions::SymbolList candidates;
    for (const dna::Word& symbol : dna::allWords(dna::SYMBOL_LENGTH))
        if (conditions::mayAppearInAdmissibleSet(symbol))
            candidates.push_back(symbol);
    return candidates;
}

conditions::SymbolList allowedUnder(const conditions::SymbolList& candidates,
                                    const std::vector<junctions::Split>& splits,
                                    unsigned choice) {
    conditions::SymbolList allowed;
    for (const dna::Word& symbol : candidates)
        if (junctions::allowed(symbol, splits, choice)) allowed.push_back(symbol);
    return allowed;
}

exact_cover::Solver buildSolver(const conditions::SymbolList& symbols,
                                const Columns& columns) {
    exact_cover::Solver solver(columns.size());
    for (const dna::Word& symbol : symbols) {
        std::vector<std::size_t> covered;
        for (const dna::Word& edge : dna::symbolEdges(symbol))
            covered.push_back(columns.at(dna::orbitRepresentative(edge)));
        solver.addRow(covered);
    }
    return solver;
}

}  // namespace cover_model
