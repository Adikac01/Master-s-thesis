#include "search.hpp"

#include "debruijn.hpp"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <set>
#include <thread>

namespace search {

namespace {
constexpr auto FIRST_SLICE = std::chrono::milliseconds(10);
}  // namespace

Problem makeProblem() {
    return {cover_model::orbitColumns(), cover_model::usableCandidates(),
            junctions::allSplits(),
            debruijn::Graph(dna::ALPHABET_SIZE, dna::EDGE_LENGTH).oddMirrorPairs()};
}

bool ruledOutByParity(const Problem& problem, unsigned choice) {
    std::set<dna::Word> touched;
    for (const dna::Word& symbol :
         cover_model::allowedUnder(problem.candidates, problem.splits, choice)) {
        const dna::Word start = dna::orbitRepresentative(symbol.substr(0, dna::VERTEX_LENGTH));
        const dna::Word end =
            dna::orbitRepresentative(symbol.substr(symbol.size() - dna::VERTEX_LENGTH));
        if (start != end) {
            touched.insert(start);
            touched.insert(end);
        }
    }
    for (const dna::Word& pair : problem.oddPairs)
        if (!touched.count(pair)) return true;
    return false;
}

conditions::SymbolList firstCover(const Problem& problem, unsigned choice) {
    const conditions::SymbolList symbols =
        cover_model::allowedUnder(problem.candidates, problem.splits, choice);
    exact_cover::Solver solver = cover_model::buildSolver(symbols, problem.columns);

    exact_cover::Row rows;
    if (!solver.solve(rows)) return {};

    conditions::SymbolList cover;
    cover.reserve(rows.size());
    for (const std::size_t row : rows) cover.push_back(symbols[row]);
    return cover;
}

std::size_t countCovers(const Problem& problem, unsigned choice,
                        std::size_t limit) {
    exact_cover::Solver solver = cover_model::buildSolver(
        cover_model::allowedUnder(problem.candidates, problem.splits, choice),
        problem.columns);
    std::size_t found = 0;
    solver.enumerate([&](const exact_cover::Row&) {
        ++found;
        return found < limit;
    });
    return found;
}

Outcome enumerateUntil(const Problem& problem, unsigned choice,
                       std::chrono::steady_clock::time_point deadline) {
    const conditions::SymbolList symbols =
        cover_model::allowedUnder(problem.candidates, problem.splits, choice);
    std::vector<unsigned> masks;
    masks.reserve(symbols.size());
    for (const dna::Word& symbol : symbols)
        masks.push_back(junctions::suffixMask(symbol, problem.splits));

    exact_cover::Solver solver = cover_model::buildSolver(symbols, problem.columns);
    solver.setDeadline(deadline);

    Outcome outcome;
    solver.enumerate([&](const exact_cover::Row& rows) {
        ++outcome.covers;
        // A set is counted only under the choice it determines itself,
        // so a set admissible under several choices is counted once.
        unsigned own = 0;
        for (const std::size_t row : rows) own |= masks[row];
        if (own == choice) ++outcome.distinct;
        return true;
    });
    outcome.finished = !solver.timedOut();
    return outcome;
}

std::vector<Outcome> searchAll(const Problem& problem, double seconds,
                               unsigned threads, const RoundCallback& onRound) {
    using Clock = std::chrono::steady_clock;
    threads = std::max(1u, threads);
    const unsigned choiceCount = problem.choiceCount();
    const auto end = Clock::now() + std::chrono::duration_cast<Clock::duration>(
                                        std::chrono::duration<double>(seconds));

    std::vector<Outcome> outcomes(choiceCount);
    std::vector<unsigned> open(choiceCount);
    for (unsigned choice = 0; choice < choiceCount; ++choice) open[choice] = choice;

    Clock::duration slice = FIRST_SLICE;
    for (int round = 1; !open.empty(); ++round) {
        const auto remaining = end - Clock::now();
        if (remaining <= Clock::duration::zero()) break;
        // each thread handles about this many choices in the round
        const long perThread = static_cast<long>((open.size() + threads - 1) / threads);
        const bool last = slice * perThread > remaining;
        if (last) slice = remaining / perThread;

        if (onRound)
            onRound(round, open.size(), std::chrono::duration<double>(slice).count());

        std::vector<unsigned> stillOpen;
        std::mutex guard;
        std::atomic<std::size_t> next{0};
        auto worker = [&]() {
            for (std::size_t i = next++; i < open.size(); i = next++) {
                const unsigned choice = open[i];
                const Outcome fresh = enumerateUntil(problem, choice, Clock::now() + slice);
                std::lock_guard<std::mutex> lock(guard);
                // A longer run finds everything a shorter one did, so keep the best.
                if (fresh.finished || fresh.covers >= outcomes[choice].covers)
                    outcomes[choice] = fresh;
                if (!fresh.finished) stillOpen.push_back(choice);
            }
        };
        std::vector<std::thread> pool;
        for (unsigned t = 0; t < threads; ++t) pool.emplace_back(worker);
        for (std::thread& thread : pool) thread.join();

        std::sort(stillOpen.begin(), stillOpen.end());
        open.swap(stillOpen);
        if (last) break;
        slice *= 2;
    }
    return outcomes;
}

}  // namespace search
