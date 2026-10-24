#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "conditions.hpp"
#include "cover_model.hpp"
#include "junctions.hpp"

#include <chrono>
#include <cstddef>
#include <functional>
#include <vector>

namespace search {

struct Problem {
    cover_model::Columns columns;
    conditions::SymbolList candidates;
    std::vector<junctions::Split> splits;
    std::vector<dna::Word> oddPairs;  // mirror pairs that cannot be balanced

    unsigned choiceCount() const { return 1u << splits.size(); }
};

Problem makeProblem();

// True if the parity argument alone rules the choice out: some mirror pair
// that cannot be balanced has no allowed candidate with exactly one end in it.
bool ruledOutByParity(const Problem& problem, unsigned choice);

// Returns an empty list if this choice admits no cover.
conditions::SymbolList firstCover(const Problem& problem, unsigned choice);

// Stops at the limit, so the result is only a lower bound.
std::size_t countCovers(const Problem& problem, unsigned choice,
                        std::size_t limit);

struct Outcome {
    unsigned long long covers = 0;
    unsigned long long distinct = 0;
    bool finished = false;
};

// With a nonzero seed the rows are searched in an order shuffled by that seed.
// A finished search is complete whatever the order.
Outcome enumerateUntil(const Problem& problem, unsigned choice,
                       std::chrono::steady_clock::time_point deadline,
                       unsigned seed = 0);

// Called at the start of each round with the choices still open and the
// time slice (in seconds) each of them gets.
using RoundCallback =
    std::function<void(int round, std::size_t open, double slice)>;

// Searches every choice within the given time, in rounds: each open choice
// gets the same slice, which doubles from round to round, and choices that
// finish drop out. Every round uses a new row order, so a choice whose search
// got stuck in a branch without covers gets another chance.
std::vector<Outcome> searchAll(const Problem& problem, double seconds,
                               unsigned threads,
                               const RoundCallback& onRound = {});

}  // namespace search

#endif
