// Conflict graph and the largest admissible subset (section 3.8).
#include "conditions.hpp"
#include "conflicts.hpp"
#include "dna.hpp"
#include "testing.hpp"

#include <cstddef>
#include <vector>

namespace {

using conditions::SymbolList;

SymbolList fixture() {
    return {"AGTTTA", "CGCCGC", "CTTAGT", "TATCGC"};
}

// The appendix of Chattopadhyay & Chakrabarti (2022), kept here so the test reads no files.
SymbolList chattopadhyay() {
    return {"CGAAAA", "ATGGAA", "CCTGAA", "TGACAA", "TCGTAA", "ACCAGA", "CGAGGA", "GACGGA",
            "GAACGA", "ATCCGA", "CTATGA", "ATGTGA", "TAAACA", "TAGCCA", "CGCCCA", "GATTCA",
            "CAGGTA", "GGTGTA", "TAACTA", "TCCCTA", "CCTCTA", "GTATTA", "TAGTTA", "CGGCAG",
            "ACTCAG", "AAATAG", "CAGTAG", "TCTTAG", "GCAAGG", "AGTGGG", "GGTTGG", "CGCACG",
            "ACAGCG", "GGGGCG", "AATGCG", "CGACCG", "ATCTCG", "GTTTCG", "GCGATG", "CTTCTG"};
}

conflicts::Graph cycle(std::size_t length) {
    conflicts::Graph graph(length, std::vector<bool>(length, false));
    for (std::size_t v = 0; v < length; ++v) {
        const std::size_t next = (v + 1) % length;
        graph[v][next] = graph[next][v] = true;
    }
    return graph;
}

bool independent(const conflicts::Graph& graph, const std::vector<std::size_t>& vertices) {
    for (std::size_t u : vertices)
        for (std::size_t v : vertices)
            if (graph[u][v]) return false;
    return true;
}

}  // namespace

int main() {
    Suite suite("conflicts");
    const SymbolList valid = fixture();

    bool noneInConflict = true;
    for (std::size_t i = 0; i < valid.size(); ++i)
        for (std::size_t j = i + 1; j < valid.size(); ++j)
            noneInConflict = noneInConflict && !conflicts::inConflict(valid[i], valid[j]);
    suite.expect("symbols of a valid set are not in conflict", noneInConflict);
    suite.expect("a shared rho-orbit is a conflict",
                 conflicts::inConflict(valid[0], dna::reverseComplement(valid[0])));
    suite.expect("a recognition site across a junction is a conflict in both orders",
                 conflicts::inConflict("TTGCAG", "CAATTC") && conflicts::inConflict("CAATTC", "TTGCAG"));

    const conflicts::ConflictGraph withBad = conflicts::build({"CGGCAG", valid[0]});
    suite.expect("a symbol whose square contains a site is rejected",
                 withBad.rejected == SymbolList{"CGGCAG"} && withBad.usable == SymbolList{valid[0]});

    const conflicts::Graph empty(3, std::vector<bool>(3, false));
    suite.expect("a graph without conflicts keeps every vertex",
                 conflicts::largestIndependentSet(empty).size() == 3);
    suite.expect("a cycle of length 5 keeps 2 vertices",
                 conflicts::largestIndependentSet(cycle(5)).size() == 2);
    const std::vector<std::size_t> fromSeven = conflicts::largestIndependentSet(cycle(7));
    suite.expect("a cycle of length 7 keeps 3 vertices, none adjacent",
                 fromSeven.size() == 3 && independent(cycle(7), fromSeven));

    const conflicts::ConflictGraph published = conflicts::build(chattopadhyay());
    suite.expect("the published set has one unusable symbol and 16 conflicts",
                 published.rejected.size() == 1 && published.conflictCount() == 16);
    const SymbolList subset = conflicts::largestAdmissibleSubset(published);
    suite.expect("at most 29 of its symbols can be kept", subset.size() == 29);
    suite.expect("the kept symbols satisfy (A), (B1) and (B2)",
                 conditions::noSharedOrbit(subset).passed &&
                 conditions::noSiteWithinSymbols(subset).passed &&
                 conditions::noSiteAcrossJunctions(subset).passed);

    return suite.finish();
}
