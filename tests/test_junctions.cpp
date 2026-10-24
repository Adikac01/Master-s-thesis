// Splits of a recognition site across a junction (section 3.5).
#include "junctions.hpp"
#include "testing.hpp"

#include <vector>

int main() {
    Suite suite("junctions");
    const std::vector<junctions::Split> splits = junctions::allSplits();

    // the order of Table 3.1: GCAGC for j = 4..1, then GCTGC for j = 4..1
    const std::vector<std::pair<dna::Word, dna::Word>> expected = {
        {"GCAG", "C"}, {"GCA", "GC"}, {"GC", "AGC"}, {"G", "CAGC"},
        {"GCTG", "C"}, {"GCT", "GC"}, {"GC", "TGC"}, {"G", "CTGC"}};
    bool sameOrder = splits.size() == expected.size();
    for (std::size_t i = 0; sameOrder && i < splits.size(); ++i)
        sameOrder = splits[i].suffix == expected[i].first &&
                    splits[i].prefix == expected[i].second;
    suite.expect("the eight splits follow Table 3.1", sameOrder);

    suite.expect("choice 0 bans every suffix, e.g. a final G",
                 !junctions::allowed("AAAAAG", splits, 0));
    suite.expect("choice 0 lets through a symbol ending with T",
                 junctions::allowed("AAAAAT", splits, 0));
    suite.expect("choice 1 bans the prefix C of the first split",
                 !junctions::allowed("CAAAAT", splits, 1));
    suite.expect("choice 0 does not ban that prefix",
                 junctions::allowed("CAAAAT", splits, 0));

    suite.expect("AAAGCA ends with the suffix of split 1 only",
                 junctions::suffixMask("AAAGCA", splits) == (1u << 1));
    suite.expect("AAAAGC ends with the suffixes of splits 2 and 6",
                 junctions::suffixMask("AAAAGC", splits) == ((1u << 2) | (1u << 6)));
    suite.expect("AAAAAT ends with no suffix", junctions::suffixMask("AAAAAT", splits) == 0);

    // TTGCAG + CAATTC contains GCAGC, so no choice may admit both symbols.
    bool someChoiceAdmitsBoth = false;
    for (unsigned choice = 0; choice < (1u << splits.size()); ++choice)
        if (junctions::allowed("TTGCAG", splits, choice) &&
            junctions::allowed("CAATTC", splits, choice))
            someChoiceAdmitsBoth = true;
    suite.expect("no choice admits a pair that forms a site across the junction",
                 !someChoiceAdmitsBoth);

    return suite.finish();
}
