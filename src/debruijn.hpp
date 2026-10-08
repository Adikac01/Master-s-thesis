#ifndef DEBRUIJN_HPP
#define DEBRUIJN_HPP

// De Bruijn graphs B(k, n) over an alphabet with a complementing involution.
// B(4,4) over the nucleotides is one instance.

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace debruijn {

using Word = std::string;

// Letter i is complementary to letter size-1-i, so the size has to be even;
// "ACGT" gives A-T and C-G.
class Alphabet {
public:
    explicit Alphabet(std::string letters);

    std::size_t size() const { return letters_.size(); }
    const std::string& letters() const { return letters_; }

    char complement(char letter) const;              // sigma
    Word complement(const Word& word) const;         // Xi
    Word reverseComplement(const Word& word) const;  // rho
    Word orbitRepresentative(const Word& word) const;
    bool isSelfComplementary(const Word& word) const;

    std::vector<Word> allWords(std::size_t length) const;
    std::size_t countOrbits(std::size_t length) const;  // two-element orbits of rho
    std::size_t countFixedPoints(std::size_t length) const;

private:
    std::string letters_;
};

class Graph {
public:
    // The alphabet is the first k letters of "ACGTUVWXYZ".
    Graph(std::size_t alphabetSize, std::size_t edgeLength);

    const Alphabet& alphabet() const { return alphabet_; }
    std::size_t edgeLength() const { return edgeLength_; }
    std::size_t vertexLength() const { return edgeLength_ - 1; }
    std::size_t vertexCount() const;
    std::size_t edgeCount() const;

    std::size_t edgeOrbitCount() const;
    std::size_t selfComplementaryEdgeCount() const;
    std::size_t vertexPairCount() const;
    std::size_t selfComplementaryVertexCount() const;

    // Representatives of the mirror pairs met by an odd number of orbits;
    // their balance is odd in every transversal (section 4.4).
    std::vector<Word> oddMirrorPairs() const;

    // Lower bound on the sum of |outdeg - indeg| over any transversal.
    std::size_t imbalanceLowerBound() const;

private:
    Alphabet alphabet_;
    std::size_t edgeLength_;
};

// An edge runs from its prefix to its suffix, both one letter shorter.
Word source(const Word& edge);
Word target(const Word& edge);

using DegreeMap = std::map<Word, int>;
DegreeMap inDegrees(const std::vector<Word>& edges);
DegreeMap outDegrees(const std::vector<Word>& edges);

struct SubgraphStats {
    std::size_t vertices = 0;
    std::size_t unbalanced = 0;
    std::size_t imbalance = 0;  // sum of |outdeg - indeg|
    bool weaklyConnected = false;
};

SubgraphStats analyseSubgraph(const std::vector<Word>& edges);

}  // namespace debruijn

#endif
