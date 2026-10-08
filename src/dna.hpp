#ifndef DNA_HPP
#define DNA_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace dna {

using Word = std::string;

inline constexpr char ALPHABET[] = "ACGT";
inline constexpr std::size_t ALPHABET_SIZE = 4;
inline constexpr std::size_t VERTEX_LENGTH = 3;
inline constexpr std::size_t EDGE_LENGTH = 4;
inline constexpr std::size_t SYMBOL_LENGTH = 6;
inline constexpr std::size_t EDGES_PER_SYMBOL = 3;

extern const Word RECOGNITION_SITE;          // GCAGC, recognised by BbvI
extern const Word RECOGNITION_SITE_REVCOMP;  // GCTGC

char complementBase(char base);            // sigma
Word complement(const Word& word);         // Xi
Word reversed(const Word& word);           // x^-1
Word reverseComplement(const Word& word);  // rho
Word orbitRepresentative(const Word& word);
bool isSelfComplementary(const Word& word);

bool contains(const Word& text, const Word& pattern);
bool containsRecognitionSite(const Word& text);

Word edgeSource(const Word& edge);
Word edgeTarget(const Word& edge);
std::vector<Word> symbolEdges(const Word& symbol);
bool formsWalk(const std::vector<Word>& edges);

std::vector<Word> allWords(std::size_t length);
std::size_t countOrbits(std::size_t length);
std::size_t countFixedPoints(std::size_t length);

}  // namespace dna

#endif
