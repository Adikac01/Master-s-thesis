// Structure of B(k,n) used in chapter 4.
#include "debruijn.hpp"
#include "testing.hpp"

#include <algorithm>
#include <stdexcept>
#include <tuple>

int main() {
    Suite suite("debruijn");

    const debruijn::Graph graph(4, 4);
    suite.expect("B(4,4) has 64 vertices and 256 edges",
                 graph.vertexCount() == 64 && graph.edgeCount() == 256);
    suite.expect("rho has 120 two-element orbits and 16 fixed points on edges",
                 graph.edgeOrbitCount() == 120 && graph.selfComplementaryEdgeCount() == 16);
    suite.expect("rho has 32 mirror pairs and no fixed points on vertices",
                 graph.vertexPairCount() == 32 && graph.selfComplementaryVertexCount() == 0);
    suite.expect("no transversal of B(4,4) has imbalance below 12",
                 graph.imbalanceLowerBound() == 12);

    // Table 4.2: e.g. {AAT, ATT} is odd, while {ATA, TAT} carries two
    // self-complementary edges and is even.
    const std::vector<debruijn::Word> odd = graph.oddMirrorPairs();
    const auto has = [&](const debruijn::Word& pair) {
        return std::find(odd.begin(), odd.end(), pair) != odd.end();
    };
    suite.expect("the odd mirror pairs are those of Table 4.2",
                 odd.size() == 12 && has("AAT") && has("AGC") && !has("ATA") && !has("CGC"));
    suite.expect("for k = 4 the alphabet pairs A-T and C-G",
                 graph.alphabet().complement('A') == 'T' && graph.alphabet().complement('C') == 'G');

    // k^(n/2) - k for even n, 0 for odd n
    bool formulaHolds = true;
    for (const auto& [k, n, bound] : {std::tuple<int, int, int>{2, 4, 2}, {2, 6, 6},
                                      {4, 6, 60}, {6, 4, 30}, {4, 5, 0}, {2, 7, 0}})
        formulaHolds = formulaHolds && debruijn::Graph(k, n).imbalanceLowerBound() ==
                                           static_cast<std::size_t>(bound);
    suite.expect("the bound equals k^(n/2) - k for even n and 0 for odd n", formulaHolds);

    // The walk of GATTCA: GAT -> ATT -> TTC -> TCA.
    const debruijn::SubgraphStats walk = debruijn::analyseSubgraph({"GATT", "ATTC", "TTCA"});
    suite.expect("the walk of GATTCA touches 4 vertices and is weakly connected",
                 walk.vertices == 4 && walk.weaklyConnected);
    suite.expect("only its two ends are unbalanced, by one each",
                 walk.unbalanced == 2 && walk.imbalance == 2);
    const debruijn::SubgraphStats apart = debruijn::analyseSubgraph({"AAAC", "GGGT"});
    suite.expect("two edges with no common vertex are not connected", !apart.weaklyConnected);
    const debruijn::DegreeMap in = debruijn::inDegrees({"GATT", "CATT"});
    suite.expect("in-degrees count edges ending at a vertex", in.at("ATT") == 2);

    bool rejected = false;
    try {
        debruijn::Graph(3, 4);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    suite.expect("an odd alphabet size is rejected", rejected);

    return suite.finish();
}
