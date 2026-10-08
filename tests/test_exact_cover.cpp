#include "exact_cover.hpp"
#include "testing.hpp"

#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace {

// Knuth's example: the only exact cover is rows 1, 3 and 5.
exact_cover::Solver knuthExample() {
    exact_cover::Solver solver(7);
    solver.addRow({0, 3, 6});
    solver.addRow({0, 3});
    solver.addRow({3, 4, 6});
    solver.addRow({2, 4, 5});
    solver.addRow({1, 2, 5, 6});
    solver.addRow({1, 6});
    return solver;
}

template <typename Exception, typename Action>
bool throws(Action action) {
    try {
        action();
    } catch (const Exception&) {
        return true;
    }
    return false;
}

}  // namespace

int main() {
    Suite suite("exact_cover");

    exact_cover::Solver solver = knuthExample();
    exact_cover::Row cover;
    suite.expect("finds a cover for Knuth's example", solver.solve(cover));
    std::sort(cover.begin(), cover.end());
    suite.expect("the cover is exactly rows 1, 3, 5", cover == exact_cover::Row{1, 3, 5});

    exact_cover::Solver counter = knuthExample();
    std::size_t total = 0;
    counter.enumerate([&](const exact_cover::Row&) { ++total; return true; });
    suite.expect("the example has exactly one cover", total == 1);

    exact_cover::Solver impossible(3);  // column 2 cannot be covered
    impossible.addRow({0});
    impossible.addRow({1});
    exact_cover::Row unused;
    suite.expect("reports failure when no cover exists", !impossible.solve(unused));

    exact_cover::Solver late = knuthExample();
    late.setDeadline(std::chrono::steady_clock::now() - std::chrono::seconds(1));
    exact_cover::Row none;
    const bool foundLate = late.solve(none);
    suite.expect("stops at once when the deadline has passed", !foundLate && late.timedOut());

    exact_cover::Solver noDeadline = knuthExample();
    exact_cover::Row some;
    suite.expect("without a deadline the search is not reported as timed out",
                 noDeadline.solve(some) && !noDeadline.timedOut());

    // Column 2 is secondary: {0,1} covers the primaries alone, while
    // {0,2} and {1,2} together would use column 2 twice.
    exact_cover::Solver optional(2, 1);
    optional.addRow({0, 1});
    optional.addRow({0, 2});
    optional.addRow({1, 2});
    std::size_t optionalCovers = 0;
    exact_cover::Row lastCover;
    optional.enumerate([&](const exact_cover::Row& c) {
        ++optionalCovers;
        lastCover = c;
        return true;
    });
    suite.expect("a secondary column may stay uncovered but is never covered twice",
                 optionalCovers == 1 && lastCover == exact_cover::Row{0});

    exact_cover::Solver stuck(3);  // {0,1} leaves column 2 uncoverable
    stuck.addRow({0, 1});
    stuck.addRow({1, 2});
    exact_cover::Row noCover;
    const bool foundStuck = stuck.solve(noCover);
    suite.expect("a failed search remembers its deepest partial cover",
                 !foundStuck && stuck.deepest().size() == 1);

    exact_cover::Solver strict(3);
    suite.expect("a column out of range is rejected",
                 throws<std::out_of_range>([&] { strict.addRow({0, 3}); }));
    suite.expect("a column repeated in a row is rejected",
                 throws<std::invalid_argument>([&] { strict.addRow({1, 1}); }));
    strict.addRow({0, 1, 2});
    exact_cover::Row after;
    suite.expect("a rejected row leaves the matrix usable", strict.solve(after));

    return suite.finish();
}
