// Operations on words, checked against the examples in chapter 2.
#include "dna.hpp"
#include "testing.hpp"

#include <vector>

int main() {
    Suite suite("dna");

    suite.expect("sigma pairs A with T and C with G",
                 dna::complementBase('A') == 'T' && dna::complementBase('T') == 'A' &&
                 dna::complementBase('C') == 'G' && dna::complementBase('G') == 'C');

    // x = GCAGC: Xi(x) = CGTCG, x^-1 = CGACG
    suite.expect("Xi(GCAGC) = CGTCG", dna::complement("GCAGC") == "CGTCG");
    suite.expect("GCAGC reversed is CGACG", dna::reversed("GCAGC") == "CGACG");
    suite.expect("rho(GCAGC) = GCTGC", dna::reverseComplement("GCAGC") == "GCTGC");

    suite.expect("AATT is self-complementary", dna::isSelfComplementary("AATT"));
    suite.expect("GATT is not self-complementary", !dna::isSelfComplementary("GATT"));
    suite.expect("16 self-complementary words of length 4", dna::countFixedPoints(4) == 16);
    suite.expect("120 two-element orbits of length 4", dna::countOrbits(4) == 120);

    const std::vector<dna::Word> edges = dna::symbolEdges("GATTCA");
    suite.expect("GATTCA has edges GATT, ATTC, TTCA",
                 edges == std::vector<dna::Word>{"GATT", "ATTC", "TTCA"});
    suite.expect("the edges of GATTCA form a walk", dna::formsWalk(edges));
    suite.expect("orbit representatives of GATTCA are AATC, ATTC, TGAA",
                 dna::orbitRepresentative("GATT") == "AATC" &&
                 dna::orbitRepresentative("ATTC") == "ATTC" &&
                 dna::orbitRepresentative("TTCA") == "TGAA");

    suite.expect("TTGCAG + CAATTC contains a recognition site",
                 dna::containsRecognitionSite("TTGCAGCAATTC"));
    suite.expect("GATTCA contains no recognition site",
                 !dna::containsRecognitionSite("GATTCA"));

    return suite.finish();
}
