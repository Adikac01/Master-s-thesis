#include "dna.hpp"

#include "debruijn.hpp"

namespace dna {

const Word RECOGNITION_SITE = "GCAGC";
const Word RECOGNITION_SITE_REVCOMP = "GCTGC";

namespace {

const debruijn::Alphabet& nucleotides() {
    static const debruijn::Alphabet alphabet(ALPHABET);
    return alphabet;
}

}  // namespace

char complementBase(char base) { return nucleotides().complement(base); }
Word complement(const Word& word) { return nucleotides().complement(word); }
Word reversed(const Word& word) { return Word(word.rbegin(), word.rend()); }
Word reverseComplement(const Word& word) { return nucleotides().reverseComplement(word); }
Word orbitRepresentative(const Word& word) { return nucleotides().orbitRepresentative(word); }
bool isSelfComplementary(const Word& word) { return nucleotides().isSelfComplementary(word); }

bool contains(const Word& text, const Word& pattern) {
    return text.find(pattern) != Word::npos;
}

bool containsRecognitionSite(const Word& text) {
    return contains(text, RECOGNITION_SITE) || contains(text, RECOGNITION_SITE_REVCOMP);
}

Word edgeSource(const Word& edge) { return debruijn::source(edge); }
Word edgeTarget(const Word& edge) { return debruijn::target(edge); }

std::vector<Word> symbolEdges(const Word& symbol) {
    std::vector<Word> edges;
    if (symbol.size() < EDGE_LENGTH) return edges;
    for (std::size_t i = 0; i + EDGE_LENGTH <= symbol.size(); ++i)
        edges.push_back(symbol.substr(i, EDGE_LENGTH));
    return edges;
}

bool formsWalk(const std::vector<Word>& edges) {
    for (std::size_t i = 1; i < edges.size(); ++i)
        if (edgeTarget(edges[i - 1]) != edgeSource(edges[i])) return false;
    return true;
}

std::vector<Word> allWords(std::size_t length) { return nucleotides().allWords(length); }
std::size_t countOrbits(std::size_t length) { return nucleotides().countOrbits(length); }
std::size_t countFixedPoints(std::size_t length) { return nucleotides().countFixedPoints(length); }

}  // namespace dna
