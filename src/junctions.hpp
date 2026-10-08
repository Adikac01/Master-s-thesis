#ifndef JUNCTIONS_HPP
#define JUNCTIONS_HPP

// Ways a recognition site can straddle the junction of two symbols (section 3.4).

#include "dna.hpp"

#include <vector>

namespace junctions {

struct Split {
    dna::Word suffix;  // end of the left symbol
    dna::Word prefix;  // start of the right symbol
};

std::vector<Split> allSplits();

// bit i of the choice: 0 bans the suffix of split i, 1 bans its prefix
bool allowed(const dna::Word& symbol, const std::vector<Split>& splits,
             unsigned choice);

// bit i set if the symbol ends with the suffix of split i
unsigned suffixMask(const dna::Word& symbol, const std::vector<Split>& splits);

}  // namespace junctions

#endif
