#include "conflicts.hpp"

#include <algorithm>
#include <set>

namespace conflicts {
namespace {

std::set<dna::Word> orbitsOf(const dna::Word& symbol) {
    std::set<dna::Word> orbits;
    for (const dna::Word& edge : dna::symbolEdges(symbol))
        orbits.insert(dna::orbitRepresentative(edge));
    return orbits;
}

// Branch and bound: take the vertex with the most conflicts or leave it out.
class Search {
public:
    explicit Search(const Graph& adjacent) : adjacent_(adjacent) {}

    std::vector<std::size_t> run() {
        std::vector<std::size_t> all(adjacent_.size());
        for (std::size_t v = 0; v < all.size(); ++v) all[v] = v;
        std::vector<std::size_t> chosen;
        branch(all, chosen);
        std::sort(best_.begin(), best_.end());
        return best_;
    }

private:
    const Graph& adjacent_;
    std::vector<std::size_t> best_;

    std::size_t degreeWithin(std::size_t v, const std::vector<std::size_t>& candidates) const {
        std::size_t degree = 0;
        for (std::size_t u : candidates)
            if (adjacent_[v][u]) ++degree;
        return degree;
    }

    void branch(const std::vector<std::size_t>& candidates, std::vector<std::size_t>& chosen) {
        if (chosen.size() + candidates.size() <= best_.size()) return;

        std::size_t pick = 0;
        std::size_t pickDegree = 0;
        for (std::size_t v : candidates) {
            const std::size_t degree = degreeWithin(v, candidates);
            if (degree > pickDegree) {
                pick = v;
                pickDegree = degree;
            }
        }
        if (pickDegree == 0) {
            best_ = chosen;
            best_.insert(best_.end(), candidates.begin(), candidates.end());
            return;
        }

        std::vector<std::size_t> rest;
        for (std::size_t u : candidates)
            if (u != pick && !adjacent_[pick][u]) rest.push_back(u);
        chosen.push_back(pick);
        branch(rest, chosen);
        chosen.pop_back();

        rest.clear();
        for (std::size_t u : candidates)
            if (u != pick) rest.push_back(u);
        branch(rest, chosen);
    }
};

}  // namespace

bool inConflict(const dna::Word& u, const dna::Word& v) {
    if (dna::containsRecognitionSite(u + v) || dna::containsRecognitionSite(v + u))
        return true;
    const std::set<dna::Word> orbitsOfU = orbitsOf(u);
    for (const dna::Word& edge : dna::symbolEdges(v))
        if (orbitsOfU.count(dna::orbitRepresentative(edge)) > 0) return true;
    return false;
}

std::size_t ConflictGraph::conflictCount() const {
    std::size_t count = 0;
    for (std::size_t i = 0; i < adjacent.size(); ++i)
        for (std::size_t j = i + 1; j < adjacent.size(); ++j)
            if (adjacent[i][j]) ++count;
    return count;
}

ConflictGraph build(const conditions::SymbolList& symbols) {
    ConflictGraph graph;
    for (const dna::Word& symbol : symbols) {
        if (conditions::mayAppearInAdmissibleSet(symbol))
            graph.usable.push_back(symbol);
        else
            graph.rejected.push_back(symbol);
    }

    const std::size_t size = graph.usable.size();
    graph.adjacent.assign(size, std::vector<bool>(size, false));
    for (std::size_t i = 0; i < size; ++i)
        for (std::size_t j = i + 1; j < size; ++j)
            if (inConflict(graph.usable[i], graph.usable[j]))
                graph.adjacent[i][j] = graph.adjacent[j][i] = true;
    return graph;
}

std::vector<std::size_t> largestIndependentSet(const Graph& adjacent) {
    return Search(adjacent).run();
}

conditions::SymbolList largestAdmissibleSubset(const ConflictGraph& graph) {
    conditions::SymbolList subset;
    for (std::size_t v : largestIndependentSet(graph.adjacent))
        subset.push_back(graph.usable[v]);
    return subset;
}

}  // namespace conflicts
