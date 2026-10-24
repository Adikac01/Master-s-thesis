#ifndef WALK_COVER_HPP
#define WALK_COVER_HPP

// Families of walks of length 3 in B(k,n) whose edges use every two-element
// rho-orbit exactly once (section 4.5).

#include "debruijn.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <functional>
#include <random>
#include <vector>

namespace walk_cover {

struct Instance {
    std::size_t edgeLength = 0;
    std::size_t columnCount = 0;                        // two-element orbits
    std::vector<debruijn::Word> walks;                  // words of length n + 2
    std::vector<std::array<std::size_t, 3>> columnsOf;  // orbits of each walk
    std::vector<std::vector<std::size_t>> rowsOf;       // walks through each orbit
};

Instance build(const debruijn::Alphabet& alphabet, std::size_t edgeLength);

// Checked from the words themselves, independently of how they were found.
bool usesEveryOrbitOnce(const debruijn::Alphabet& alphabet, std::size_t edgeLength,
                        const std::vector<debruijn::Word>& walks);

using Clock = std::chrono::steady_clock;

// Restarts the exact cover search with random row and column orders, each
// run limited to the slice. Returns a cover, or the largest partial one seen.
std::vector<std::size_t> randomRestarts(const Instance& instance, std::mt19937& rng,
                                        double slice, Clock::time_point deadline);

// Extends a partial cover by freeing the walks around an uncovered orbit and
// solving that small problem exactly; called back after every improvement.
using Progress = std::function<void(std::size_t walks)>;
bool repair(const Instance& instance, std::vector<std::size_t>& rows, std::mt19937& rng,
            Clock::time_point deadline, const Progress& onImprove = {});

}  // namespace walk_cover

#endif
