// The structural problem of section 4.5 on small graphs.
#include "debruijn.hpp"
#include "testing.hpp"
#include "walk_cover.hpp"

#include <chrono>
#include <random>
#include <vector>

namespace {

std::vector<debruijn::Word> wordsOf(const walk_cover::Instance& instance,
                                    const std::vector<std::size_t>& rows) {
    std::vector<debruijn::Word> words;
    for (const std::size_t row : rows) words.push_back(instance.walks[row]);
    return words;
}

}  // namespace

int main() {
    Suite suite("walk_cover");
    const debruijn::Alphabet dna("ACGT");
    const auto soon = [] { return walk_cover::Clock::now() + std::chrono::seconds(10); };

    const walk_cover::Instance b44 = walk_cover::build(dna, 4);
    suite.expect("B(4,4) has 120 orbits and 3336 candidate walks",
                 b44.columnCount == 120 && b44.walks.size() == 3336);
    const walk_cover::Instance b46 = walk_cover::build(dna, 6);
    suite.expect("B(4,6) has 2016 orbits and 62472 candidate walks",
                 b46.columnCount == 2016 && b46.walks.size() == 62472);

    std::mt19937 rng(1);
    std::vector<std::size_t> rows = walk_cover::randomRestarts(b44, rng, 0.1, soon());
    suite.expect("random restarts find 40 walks covering B(4,4)",
                 rows.size() == 40 && walk_cover::usesEveryOrbitOnce(dna, 4, wordsOf(b44, rows)));

    std::vector<debruijn::Word> words = wordsOf(b44, rows);
    words.push_back(words.front());
    suite.expect("a complete family with an extra copy of a walk is rejected",
                 !walk_cover::usesEveryOrbitOnce(dna, 4, words));
    words.pop_back();
    words.back() = words.front();
    suite.expect("a repeated walk is rejected", !walk_cover::usesEveryOrbitOnce(dna, 4, words));
    words.pop_back();
    suite.expect("a missing walk is rejected", !walk_cover::usesEveryOrbitOnce(dna, 4, words));

    rows.resize(rows.size() - 3);
    const bool repaired = walk_cover::repair(b44, rows, rng, soon());
    suite.expect("repair completes a cover with three walks removed",
                 repaired && walk_cover::usesEveryOrbitOnce(dna, 4, wordsOf(b44, rows)));

    return suite.finish();
}
