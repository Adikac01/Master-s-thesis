// Every check has to accept a valid set and reject a set built to break it.
#include "conditions.hpp"
#include "dna.hpp"
#include "testing.hpp"

#include <utility>
#include <vector>

namespace {

using conditions::SymbolList;

// Four valid symbols, so the tests do not depend on any data file.
SymbolList fixture() {
    return {"AGTTTA", "CGCCGC", "CTTAGT", "TATCGC"};
}

SymbolList replaced(std::size_t index, const dna::Word& symbol) {
    SymbolList symbols = fixture();
    symbols[index] = symbol;
    return symbols;
}

}  // namespace

int main() {
    Suite suite("conditions");
    const SymbolList valid = fixture();

    // transversalComplete is skipped: a fixture cannot cover all 120 orbits.
    bool allPass = true;
    for (const conditions::CheckResult& result :
         {conditions::symbolsWellFormed(valid), conditions::symbolsDistinct(valid),
          conditions::walksConsistent(valid), conditions::edgesDistinct(valid),
          conditions::noSelfComplementaryEdges(valid), conditions::noSharedOrbit(valid),
          conditions::noSiteWithinSymbols(valid), conditions::noSiteAcrossJunctions(valid)})
        allPass = allPass && result.passed;
    suite.expect("a valid fixture passes every applicable check", allPass);

    suite.expect("a duplicated symbol is rejected",
                 !conditions::symbolsDistinct(replaced(1, valid[0])).passed);
    suite.expect("a recognition site inside a symbol is rejected",
                 !conditions::noSiteWithinSymbols(replaced(0, "GCAGCT")).passed);

    SymbolList junction = fixture();
    junction[0] = "TTGCAG";  // neither symbol contains a site,
    junction[1] = "CAATTC";  // but their concatenation does
    suite.expect("a recognition site across a junction is rejected",
                 !conditions::noSiteAcrossJunctions(junction).passed);

    suite.expect("a self-complementary edge is rejected",  // middle edge AATT
                 !conditions::noSelfComplementaryEdges(replaced(2, "AAATTA")).passed);
    suite.expect("two edges in one rho-orbit are rejected",
                 !conditions::noSharedOrbit(replaced(1, dna::reverseComplement(valid[0]))).passed);
    suite.expect("an incomplete transversal is rejected",  // 12 of the 120 orbits
                 !conditions::transversalComplete(valid).passed);

    // Balance is a condition on the partition, so break the partition itself.
    std::vector<SymbolList> layers = conditions::layersOf(valid);
    suite.expect("the layers of a valid set are balanced",
                 conditions::partitionBalanced(layers[0], layers[1], layers[2]).passed);
    std::swap(layers[0][0], layers[1][0]);
    suite.expect("an unbalanced partition into layers is rejected",
                 !conditions::partitionBalanced(layers[0], layers[1], layers[2]).passed);

    suite.expect("correct complementary strands are accepted",
                 conditions::strandsComplementary({{"GATTCA", "CTAAGT"}}).passed);
    suite.expect("a wrong complementary strand is rejected",
                 !conditions::strandsComplementary({{"GATTCA", "CTAAGA"}}).passed);

    return suite.finish();
}
