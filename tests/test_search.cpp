#include "conditions.hpp"
#include "cover_model.hpp"
#include "exact_cover.hpp"
#include "junctions.hpp"
#include "search.hpp"
#include "testing.hpp"

#include <chrono>

namespace {

using Clock = std::chrono::steady_clock;

unsigned ownChoice(const conditions::SymbolList& symbols,
                   const std::vector<junctions::Split>& splits) {
    unsigned own = 0;
    for (const dna::Word& symbol : symbols) own |= junctions::suffixMask(symbol, splits);
    return own;
}

// Checks the first covers found under a choice: each one must be allowed
// under its own choice, so that it is counted exactly once there.
bool countedOnceProperly(const search::Problem& problem, unsigned choice,
                         bool expectOwn) {
    const conditions::SymbolList symbols =
        cover_model::allowedUnder(problem.candidates, problem.splits, choice);
    exact_cover::Solver solver = cover_model::buildSolver(symbols, problem.columns);
    bool ok = true;
    std::size_t seen = 0;
    solver.enumerate([&](const exact_cover::Row& rows) {
        conditions::SymbolList cover;
        for (const std::size_t row : rows) cover.push_back(symbols[row]);
        const unsigned own = ownChoice(cover, problem.splits);
        ok = ok && (own & ~choice) == 0 && ((own == choice) == expectOwn);
        for (const dna::Word& symbol : cover)
            ok = ok && junctions::allowed(symbol, problem.splits, own);
        return ++seen < 200;
    });
    return ok && seen > 0;
}

}  // namespace

int main() {
    Suite suite("search");
    const search::Problem problem = search::makeProblem();

    suite.expect("120 columns, 3306 candidates, 8 splits, 256 choices",
                 problem.columns.size() == 120 && problem.candidates.size() == 3306 &&
                 problem.splits.size() == 8 && problem.choiceCount() == 256);

    // Choice 204 is the one the published set satisfies.
    const conditions::SymbolList cover = search::firstCover(problem, 204);
    bool allChecks = cover.size() == 40;
    for (const conditions::CheckResult& result : conditions::verifyAll(cover))
        allChecks = allChecks && result.passed;
    suite.expect("choice 204 yields 40 symbols passing every check", allChecks);
    suite.expect("those symbols determine choice 204 themselves",
                 ownChoice(cover, problem.splits) == 204);

    suite.expect("choice 1 admits no cover", search::firstCover(problem, 1).empty());
    const search::Outcome empty =
        search::enumerateUntil(problem, 1, Clock::now() + std::chrono::seconds(10));
    suite.expect("searching choice 1 finishes with no cover",
                 empty.finished && empty.covers == 0);

    // In the natural order the search under choice 68 gets stuck for hours,
    // while a shuffled order finds a cover almost at once.
    bool foundShuffled = false;
    for (unsigned seed = 1; seed <= 5 && !foundShuffled; ++seed)
        foundShuffled = search::enumerateUntil(problem, 68, Clock::now() + std::chrono::seconds(2),
                                               seed).covers > 0;
    suite.expect("a shuffled search finds a cover under choice 68", foundShuffled);
    const search::Outcome shuffledEmpty =
        search::enumerateUntil(problem, 1, Clock::now() + std::chrono::seconds(10), 3);
    suite.expect("shuffling does not change the outcome of a complete search",
                 shuffledEmpty.finished && shuffledEmpty.covers == 0);

    const search::Outcome cut =
        search::enumerateUntil(problem, 0, Clock::now() + std::chrono::milliseconds(10));
    suite.expect("a search past its deadline is reported as unfinished", !cut.finished);

    suite.expect("covers under choice 204 are all counted there",
                 countedOnceProperly(problem, 204, true));
    suite.expect("covers under choice 255 all belong to other choices",
                 countedOnceProperly(problem, 255, false));
    const search::Outcome foreign =
        search::enumerateUntil(problem, 255, Clock::now() + std::chrono::seconds(1));
    suite.expect("so choice 255 contributes no distinct sets",
                 foreign.covers > 0 && foreign.distinct == 0);

    unsigned ruledOut = 0;
    bool ruledOutHaveNoCover = true;
    for (unsigned choice = 0; choice < problem.choiceCount(); ++choice)
        if (search::ruledOutByParity(problem, choice)) {
            ++ruledOut;
            ruledOutHaveNoCover = ruledOutHaveNoCover && search::firstCover(problem, choice).empty();
        }
    suite.expect("parity alone rules out 158 of the 256 choices", ruledOut == 158);
    suite.expect("no choice ruled out by parity has a cover", ruledOutHaveNoCover);
    // On the real data both ends or one end make no difference, so check the
    // rule itself: a symbol starting and ending in the same pair leaves its
    // balance unchanged and cannot help.
    search::Problem tiny;
    tiny.oddPairs = {"AAT"};  // the pair {AAT, ATT}
    tiny.candidates = {"AATATT"};
    suite.expect("a symbol with both ends in an odd pair does not balance it",
                 search::ruledOutByParity(tiny, 0));
    tiny.candidates = {"AATCCC"};
    suite.expect("a symbol with exactly one end in it does",
                 !search::ruledOutByParity(tiny, 0));
    suite.expect("choices 0 and 204 are not ruled out by parity",
                 !search::ruledOutByParity(problem, 0) && !search::ruledOutByParity(problem, 204));

    // Only timing-independent facts: how far a short run gets depends on the machine.
    const std::vector<search::Outcome> all = search::searchAll(problem, 0.3, 0);
    bool consistent = all.size() == 256 && all[1].covers == 0;
    for (const search::Outcome& outcome : all)
        consistent = consistent && outcome.distinct <= outcome.covers;
    suite.expect("searchAll covers all 256 choices even with 0 threads requested",
                 consistent);

    return suite.finish();
}
