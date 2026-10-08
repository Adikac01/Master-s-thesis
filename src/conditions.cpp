#include "conditions.hpp"

#include "debruijn.hpp"

#include <set>

namespace conditions {
namespace {

using dna::Word;

CheckResult pass(std::string name) {
    return CheckResult{std::move(name), true, ""};
}

CheckResult verdict(std::string name, bool passed, std::string detail = "") {
    return CheckResult{std::move(name), passed, std::move(detail)};
}

// Every edge of every symbol, with multiplicity.
std::vector<Word> collectEdges(const SymbolList& symbols) {
    std::vector<Word> edges;
    edges.reserve(symbols.size() * dna::EDGES_PER_SYMBOL);
    for (const Word& symbol : symbols)
        for (const Word& edge : dna::symbolEdges(symbol))
            edges.push_back(edge);
    return edges;
}

CheckResult compareDegrees(const SymbolList& incoming,
                           const SymbolList& outgoing,
                           std::string name) {
    const debruijn::DegreeMap in = debruijn::inDegrees(incoming);
    const debruijn::DegreeMap out = debruijn::outDegrees(outgoing);

    std::set<Word> vertices;
    for (const auto& [vertex, unused] : in)  vertices.insert(vertex);
    for (const auto& [vertex, unused] : out) vertices.insert(vertex);

    std::string mismatches;
    for (const Word& vertex : vertices) {
        const auto inIt  = in.find(vertex);
        const auto outIt = out.find(vertex);
        const int inDegree  = (inIt  != in.end())  ? inIt->second  : 0;
        const int outDegree = (outIt != out.end()) ? outIt->second : 0;
        if (inDegree != outDegree)
            mismatches += " " + vertex + "(" + std::to_string(inDegree) +
                          "!=" + std::to_string(outDegree) + ")";
    }
    return verdict(std::move(name), mismatches.empty(), mismatches);
}

}  // namespace

bool mayAppearInAdmissibleSet(const Word& symbol) {
    if (symbol.size() != dna::SYMBOL_LENGTH) return false;
    if (dna::containsRecognitionSite(symbol)) return false;
    if (dna::containsRecognitionSite(symbol + symbol)) return false;

    const std::vector<Word> edges = dna::symbolEdges(symbol);
    std::set<Word> orbits;
    for (const Word& edge : edges) {
        if (dna::isSelfComplementary(edge)) return false;
        orbits.insert(dna::orbitRepresentative(edge));
    }
    return orbits.size() == edges.size();
}

std::vector<SymbolList> layersOf(const SymbolList& symbols) {
    std::vector<SymbolList> layers(dna::EDGES_PER_SYMBOL);
    for (const Word& symbol : symbols) {
        const std::vector<Word> edges = dna::symbolEdges(symbol);
        for (std::size_t i = 0; i < edges.size() && i < layers.size(); ++i)
            layers[i].push_back(edges[i]);
    }
    return layers;
}

CheckResult symbolsWellFormed(const SymbolList& symbols) {
    std::string offenders;
    for (std::size_t i = 0; i < symbols.size(); ++i)
        if (symbols[i].size() != dna::SYMBOL_LENGTH)
            offenders += " #" + std::to_string(i + 1);
    return verdict("every symbol has length " + std::to_string(dna::SYMBOL_LENGTH),
                   offenders.empty(), offenders);
}

CheckResult symbolsDistinct(const SymbolList& symbols) {
    const std::set<Word> unique(symbols.begin(), symbols.end());
    const bool ok = unique.size() == symbols.size();
    return verdict("symbols are pairwise distinct", ok,
                   ok ? "" : "distinct: " + std::to_string(unique.size()) +
                             " of " + std::to_string(symbols.size()));
}

CheckResult walksConsistent(const SymbolList& symbols) {
    std::string offenders;
    for (std::size_t i = 0; i < symbols.size(); ++i)
        if (!dna::formsWalk(dna::symbolEdges(symbols[i])))
            offenders += " #" + std::to_string(i + 1);
    return verdict("consecutive edges form a walk (target = source)",
                   offenders.empty(), offenders);
}

CheckResult edgesDistinct(const SymbolList& symbols) {
    const std::vector<Word> edges = collectEdges(symbols);
    const std::set<Word> unique(edges.begin(), edges.end());
    const bool ok = unique.size() == edges.size();
    return verdict("all edges are pairwise distinct", ok,
                   "distinct: " + std::to_string(unique.size()) + " of " +
                       std::to_string(edges.size()));
}

CheckResult noSelfComplementaryEdges(const SymbolList& symbols) {
    std::set<Word> offenders;
    for (const Word& edge : collectEdges(symbols))
        if (dna::isSelfComplementary(edge)) offenders.insert(edge);
    std::string detail;
    for (const Word& edge : offenders) detail += " " + edge;
    return verdict("no self-complementary edges", offenders.empty(), detail);
}

CheckResult noSharedOrbit(const SymbolList& symbols) {
    const std::vector<Word> edges = collectEdges(symbols);
    const std::set<Word> edgeSet(edges.begin(), edges.end());
    std::set<Word> clashes;
    for (const Word& edge : edgeSet)
        if (edgeSet.count(dna::reverseComplement(edge)))
            clashes.insert(dna::orbitRepresentative(edge));
    std::string detail;
    for (const Word& representative : clashes)
        detail += " {" + representative + "," +
                  dna::reverseComplement(representative) + "}";
    return verdict("no two edges lie in the same rho-orbit", clashes.empty(),
                   detail);
}

CheckResult transversalComplete(const SymbolList& symbols) {
    const std::vector<Word> edges = collectEdges(symbols);
    std::set<Word> orbits;
    for (const Word& edge : edges) orbits.insert(dna::orbitRepresentative(edge));
    const std::size_t total = dna::countOrbits(dna::EDGE_LENGTH);
    const bool ok = orbits.size() == total;
    return verdict("the edges form a complete transversal of the rho-orbits",
                   ok, "orbits used: " + std::to_string(orbits.size()) +
                           " of " + std::to_string(total));
}

CheckResult noSiteWithinSymbols(const SymbolList& symbols) {
    std::string offenders;
    for (std::size_t i = 0; i < symbols.size(); ++i)
        if (dna::containsRecognitionSite(symbols[i]))
            offenders += " #" + std::to_string(i + 1) + "(" + symbols[i] + ")";
    return verdict("no symbol contains " + dna::RECOGNITION_SITE + " or " +
                       dna::RECOGNITION_SITE_REVCOMP,
                   offenders.empty(),
                   offenders);
}

CheckResult noSiteAcrossJunctions(const SymbolList& symbols) {
    std::size_t violations = 0;
    std::string examples;
    for (std::size_t i = 0; i < symbols.size(); ++i)
        for (std::size_t j = 0; j < symbols.size(); ++j) {
            const Word junction = symbols[i] + symbols[j];
            if (!dna::containsRecognitionSite(junction)) continue;
            ++violations;
            if (violations <= 5)
                examples += " (" + std::to_string(i + 1) + "," +
                            std::to_string(j + 1) + ")=" + junction;
        }
    const std::size_t pairs = symbols.size() * symbols.size();
    return verdict("no concatenation uv contains a recognition site",
                   violations == 0,
                   violations == 0
                       ? "pairs checked: " + std::to_string(pairs)
                       : "violations: " + std::to_string(violations) + examples);
}

CheckResult partitionBalanced(const SymbolList& first,
                              const SymbolList& second,
                              const SymbolList& third) {
    const CheckResult firstPair =
        compareDegrees(first, second, "every vertex: indeg(G1) = outdeg(G2)");
    if (!firstPair.passed) return firstPair;

    const CheckResult secondPair =
        compareDegrees(second, third, "every vertex: indeg(G2) = outdeg(G3)");
    if (!secondPair.passed) return secondPair;

    return pass("balance holds for both consecutive layer pairs");
}

CheckResult strandsComplementary(const std::vector<StrandPair>& rows) {
    std::string offenders;
    for (std::size_t i = 0; i < rows.size(); ++i)
        if (dna::complement(rows[i].strand1) != rows[i].strand2)
            offenders += " #" + std::to_string(i + 1);
    return verdict("strand 2 equals Xi(strand 1) in every row",
                   offenders.empty(), offenders);
}

std::vector<CheckResult> verifyAll(const SymbolList& symbols) {
    return {
        symbolsWellFormed(symbols),
        symbolsDistinct(symbols),
        walksConsistent(symbols),
        edgesDistinct(symbols),
        noSelfComplementaryEdges(symbols),
        noSharedOrbit(symbols),
        transversalComplete(symbols),
        noSiteWithinSymbols(symbols),
        noSiteAcrossJunctions(symbols),
    };
}

}  // namespace conditions
