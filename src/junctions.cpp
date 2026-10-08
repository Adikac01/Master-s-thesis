#include "junctions.hpp"

namespace junctions {

namespace {

bool startsWith(const dna::Word& word, const dna::Word& start) {
    return word.compare(0, start.size(), start) == 0;
}

bool endsWith(const dna::Word& word, const dna::Word& end) {
    return word.size() >= end.size() &&
           word.compare(word.size() - end.size(), end.size(), end) == 0;
}

}  // namespace

std::vector<Split> allSplits() {
    std::vector<Split> splits;
    for (const dna::Word& site :
         {dna::RECOGNITION_SITE, dna::RECOGNITION_SITE_REVCOMP})
        for (std::size_t k = site.size() - 1; k >= 1; --k)
            splits.push_back({site.substr(0, k), site.substr(k)});
    return splits;
}

bool allowed(const dna::Word& symbol, const std::vector<Split>& splits,
             unsigned choice) {
    for (std::size_t i = 0; i < splits.size(); ++i) {
        const bool banPrefix = (choice >> i) & 1u;
        if (banPrefix ? startsWith(symbol, splits[i].prefix)
                      : endsWith(symbol, splits[i].suffix))
            return false;
    }
    return true;
}

unsigned suffixMask(const dna::Word& symbol, const std::vector<Split>& splits) {
    unsigned mask = 0;
    for (std::size_t i = 0; i < splits.size(); ++i)
        if (endsWith(symbol, splits[i].suffix)) mask |= 1u << i;
    return mask;
}

}  // namespace junctions
