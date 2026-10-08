#include "debruijn.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>

namespace debruijn {

namespace {

const std::string LETTERS = "ACGTUVWXYZ";

std::size_t power(std::size_t base, std::size_t exponent) {
    std::size_t result = 1;
    while (exponent-- > 0) result *= base;
    return result;
}

std::string checkedLetters(std::size_t alphabetSize) {
    if (alphabetSize > LETTERS.size())
        throw std::invalid_argument("alphabet size too large");
    return LETTERS.substr(0, alphabetSize);
}

}  // namespace

Alphabet::Alphabet(std::string letters) : letters_(std::move(letters)) {
    if (letters_.size() < 2 || letters_.size() % 2 != 0)
        throw std::invalid_argument("alphabet size must be even and at least 2");
}

char Alphabet::complement(char letter) const {
    const std::size_t index = letters_.find(letter);
    if (index == std::string::npos) return '?';
    return letters_[letters_.size() - 1 - index];
}

Word Alphabet::complement(const Word& word) const {
    Word result = word;
    for (char& letter : result) letter = complement(letter);
    return result;
}

Word Alphabet::reverseComplement(const Word& word) const {
    return complement(Word(word.rbegin(), word.rend()));
}

Word Alphabet::orbitRepresentative(const Word& word) const {
    return std::min(word, reverseComplement(word));
}

bool Alphabet::isSelfComplementary(const Word& word) const {
    return reverseComplement(word) == word;
}

std::vector<Word> Alphabet::allWords(std::size_t length) const {
    std::vector<Word> words{Word()};
    for (std::size_t position = 0; position < length; ++position) {
        std::vector<Word> extended;
        extended.reserve(words.size() * letters_.size());
        for (const Word& word : words)
            for (const char letter : letters_) extended.push_back(word + letter);
        words.swap(extended);
    }
    return words;
}

std::size_t Alphabet::countOrbits(std::size_t length) const {
    std::set<Word> representatives;
    for (const Word& word : allWords(length))
        if (!isSelfComplementary(word))
            representatives.insert(orbitRepresentative(word));
    return representatives.size();
}

std::size_t Alphabet::countFixedPoints(std::size_t length) const {
    std::size_t count = 0;
    for (const Word& word : allWords(length))
        if (isSelfComplementary(word)) ++count;
    return count;
}

Graph::Graph(std::size_t alphabetSize, std::size_t edgeLength)
    : alphabet_(checkedLetters(alphabetSize)), edgeLength_(edgeLength) {
    if (edgeLength < 2) throw std::invalid_argument("edge length must be at least 2");
}

std::size_t Graph::vertexCount() const { return power(alphabet_.size(), vertexLength()); }
std::size_t Graph::edgeCount() const { return power(alphabet_.size(), edgeLength_); }

std::size_t Graph::edgeOrbitCount() const { return alphabet_.countOrbits(edgeLength_); }

std::size_t Graph::selfComplementaryEdgeCount() const {
    return alphabet_.countFixedPoints(edgeLength_);
}

std::size_t Graph::vertexPairCount() const { return alphabet_.countOrbits(vertexLength()); }

std::size_t Graph::selfComplementaryVertexCount() const {
    return alphabet_.countFixedPoints(vertexLength());
}

std::vector<Word> Graph::oddMirrorPairs() const {
    std::map<Word, std::size_t> orbitsMeetingPair;
    std::set<Word> handled;
    for (const Word& edge : alphabet_.allWords(edgeLength_)) {
        if (alphabet_.isSelfComplementary(edge)) continue;
        const Word representative = alphabet_.orbitRepresentative(edge);
        if (!handled.insert(representative).second) continue;

        const Word sourcePair = alphabet_.orbitRepresentative(source(representative));
        const Word targetPair = alphabet_.orbitRepresentative(target(representative));
        if (sourcePair == targetPair) continue;
        ++orbitsMeetingPair[sourcePair];
        ++orbitsMeetingPair[targetPair];
    }

    std::vector<Word> odd;
    for (const auto& [pair, met] : orbitsMeetingPair)
        if (met % 2 == 1) odd.push_back(pair);
    return odd;
}

std::size_t Graph::imbalanceLowerBound() const { return oddMirrorPairs().size(); }

Word source(const Word& edge) { return edge.substr(0, edge.size() - 1); }
Word target(const Word& edge) { return edge.substr(1); }

DegreeMap inDegrees(const std::vector<Word>& edges) {
    DegreeMap degrees;
    for (const Word& edge : edges) ++degrees[target(edge)];
    return degrees;
}

DegreeMap outDegrees(const std::vector<Word>& edges) {
    DegreeMap degrees;
    for (const Word& edge : edges) ++degrees[source(edge)];
    return degrees;
}

SubgraphStats analyseSubgraph(const std::vector<Word>& edges) {
    SubgraphStats stats;
    if (edges.empty()) return stats;

    const DegreeMap in = inDegrees(edges);
    const DegreeMap out = outDegrees(edges);
    std::map<Word, std::vector<Word>> neighbours;  // ignoring direction
    for (const Word& edge : edges) {
        neighbours[source(edge)].push_back(target(edge));
        neighbours[target(edge)].push_back(source(edge));
    }

    stats.vertices = neighbours.size();
    for (const auto& [vertex, unused] : neighbours) {
        const auto inIt = in.find(vertex);
        const auto outIt = out.find(vertex);
        const int difference = (outIt != out.end() ? outIt->second : 0) -
                               (inIt != in.end() ? inIt->second : 0);
        if (difference != 0) {
            ++stats.unbalanced;
            stats.imbalance += static_cast<std::size_t>(difference < 0 ? -difference
                                                                       : difference);
        }
    }

    std::set<Word> seen;
    std::vector<Word> stack{neighbours.begin()->first};
    while (!stack.empty()) {
        const Word vertex = stack.back();
        stack.pop_back();
        if (!seen.insert(vertex).second) continue;
        for (const Word& next : neighbours.at(vertex))
            if (!seen.count(next)) stack.push_back(next);
    }
    stats.weaklyConnected = seen.size() == stats.vertices;
    return stats;
}

}  // namespace debruijn
