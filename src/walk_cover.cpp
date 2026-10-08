#include "walk_cover.hpp"

#include "exact_cover.hpp"

#include <algorithm>
#include <map>
#include <numeric>
#include <set>

namespace walk_cover {

namespace {

constexpr std::size_t FIRST_NEIGHBOURHOOD = 6;   // walks freed in one repair step
constexpr std::size_t MAX_NEIGHBOURHOOD = 60;
constexpr std::size_t FAILURES_BEFORE_GROWING = 200;
constexpr auto SUBPROBLEM_LIMIT = std::chrono::milliseconds(50);

Clock::duration seconds(double value) {
    return std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(value));
}

}  // namespace

Instance build(const debruijn::Alphabet& alphabet, std::size_t edgeLength) {
    Instance instance;
    instance.edgeLength = edgeLength;

    std::map<debruijn::Word, std::size_t> column;
    for (const debruijn::Word& edge : alphabet.allWords(edgeLength))
        if (!alphabet.isSelfComplementary(edge))
            column.emplace(alphabet.orbitRepresentative(edge), 0);
    std::size_t next = 0;
    for (auto& [orbit, index] : column) index = next++;
    instance.columnCount = column.size();
    instance.rowsOf.resize(instance.columnCount);

    for (const debruijn::Word& walk : alphabet.allWords(edgeLength + 2)) {
        std::array<std::size_t, 3> columns{};
        bool usable = true;
        for (std::size_t i = 0; i < 3 && usable; ++i) {
            const debruijn::Word edge = walk.substr(i, edgeLength);
            if (alphabet.isSelfComplementary(edge)) usable = false;
            else columns[i] = column.at(alphabet.orbitRepresentative(edge));
        }
        if (!usable || columns[0] == columns[1] || columns[0] == columns[2] ||
            columns[1] == columns[2])
            continue;
        const std::size_t row = instance.walks.size();
        instance.walks.push_back(walk);
        instance.columnsOf.push_back(columns);
        for (const std::size_t c : columns) instance.rowsOf[c].push_back(row);
    }
    return instance;
}

bool usesEveryOrbitOnce(const debruijn::Alphabet& alphabet, std::size_t edgeLength,
                        const std::vector<debruijn::Word>& walks) {
    std::set<debruijn::Word> used;
    for (const debruijn::Word& walk : walks) {
        if (walk.size() != edgeLength + 2) return false;
        for (std::size_t i = 0; i < 3; ++i) {
            const debruijn::Word edge = walk.substr(i, edgeLength);
            if (alphabet.isSelfComplementary(edge)) return false;
            if (!used.insert(alphabet.orbitRepresentative(edge)).second) return false;
        }
    }
    return used.size() == alphabet.countOrbits(edgeLength);
}

std::vector<std::size_t> randomRestarts(const Instance& instance, std::mt19937& rng,
                                        double slice, Clock::time_point deadline) {
    std::vector<std::size_t> best;
    std::vector<std::size_t> columnOrder(instance.columnCount);
    std::vector<std::size_t> rowOrder(instance.walks.size());
    std::iota(columnOrder.begin(), columnOrder.end(), 0);
    std::iota(rowOrder.begin(), rowOrder.end(), 0);

    while (Clock::now() < deadline) {
        std::shuffle(columnOrder.begin(), columnOrder.end(), rng);
        std::shuffle(rowOrder.begin(), rowOrder.end(), rng);
        exact_cover::Solver solver(instance.columnCount);
        for (const std::size_t row : rowOrder) {
            const auto& c = instance.columnsOf[row];
            solver.addRow({columnOrder[c[0]], columnOrder[c[1]], columnOrder[c[2]]});
        }
        solver.setDeadline(std::min(deadline, Clock::now() + seconds(slice)));

        exact_cover::Row cover;
        const bool complete = solver.solve(cover);
        const exact_cover::Row& found = complete ? cover : solver.deepest();
        if (found.size() > best.size()) {
            best.clear();
            for (const std::size_t i : found) best.push_back(rowOrder[i]);
        }
        if (complete) break;
    }
    return best;
}

bool repair(const Instance& instance, std::vector<std::size_t>& rows, std::mt19937& rng,
            Clock::time_point deadline, const Progress& onImprove) {
    const std::size_t target = instance.columnCount / 3;
    const long free = -1;
    std::vector<long> owner(instance.columnCount, free);
    for (const std::size_t row : rows)
        for (const std::size_t c : instance.columnsOf[row]) owner[c] = static_cast<long>(row);

    std::size_t neighbourhood = FIRST_NEIGHBOURHOOD;
    std::size_t failures = 0;
    while (rows.size() < target && Clock::now() < deadline) {
        std::vector<std::size_t> uncovered;
        for (std::size_t c = 0; c < instance.columnCount; ++c)
            if (owner[c] == free) uncovered.push_back(c);
        const std::size_t start = uncovered[rng() % uncovered.size()];

        // Free the chosen walks that block walks through the start orbit,
        // then their neighbours, until the neighbourhood is full.
        std::set<std::size_t> freed;
        std::vector<std::size_t> frontier{start};
        while (!frontier.empty() && freed.size() < neighbourhood) {
            std::swap(frontier[rng() % frontier.size()], frontier.back());
            const std::size_t column = frontier.back();
            frontier.pop_back();
            for (const std::size_t walk : instance.rowsOf[column])
                for (const std::size_t c : instance.columnsOf[walk])
                    if (owner[c] != free && freed.size() < neighbourhood &&
                        freed.insert(static_cast<std::size_t>(owner[c])).second)
                        for (const std::size_t d :
                             instance.columnsOf[static_cast<std::size_t>(owner[c])])
                            frontier.push_back(d);
        }

        // The freed orbits and the start orbit have to be covered; any other
        // uncovered orbit may be covered as a bonus.
        std::map<std::size_t, std::size_t> local;
        for (const std::size_t walk : freed)
            for (const std::size_t c : instance.columnsOf[walk]) local.emplace(c, 0);
        local.emplace(start, 0);
        std::size_t index = 0;
        for (auto& [c, i] : local) i = index++;
        const std::size_t primary = local.size();
        for (const std::size_t c : uncovered)
            if (local.emplace(c, index).second) ++index;

        std::vector<std::size_t> candidates;
        std::set<std::size_t> seen;
        for (const auto& [c, i] : local)
            for (const std::size_t walk : instance.rowsOf[c]) {
                if (!seen.insert(walk).second) continue;
                const auto& cs = instance.columnsOf[walk];
                if (local.count(cs[0]) && local.count(cs[1]) && local.count(cs[2]))
                    candidates.push_back(walk);
            }
        std::shuffle(candidates.begin(), candidates.end(), rng);

        exact_cover::Solver solver(primary, local.size() - primary);
        for (const std::size_t walk : candidates) {
            const auto& cs = instance.columnsOf[walk];
            solver.addRow({local.at(cs[0]), local.at(cs[1]), local.at(cs[2])});
        }
        solver.setDeadline(std::min(deadline, Clock::now() + SUBPROBLEM_LIMIT));
        exact_cover::Row cover;
        if (!solver.solve(cover)) {
            if (++failures % FAILURES_BEFORE_GROWING == 0 && neighbourhood < MAX_NEIGHBOURHOOD)
                neighbourhood += 2;
            continue;
        }

        for (const std::size_t walk : freed)
            for (const std::size_t c : instance.columnsOf[walk]) owner[c] = free;
        rows.erase(std::remove_if(rows.begin(), rows.end(),
                                  [&](std::size_t r) { return freed.count(r) > 0; }),
                   rows.end());
        for (const std::size_t i : cover) {
            const std::size_t walk = candidates[i];
            rows.push_back(walk);
            for (const std::size_t c : instance.columnsOf[walk]) owner[c] = static_cast<long>(walk);
        }
        neighbourhood = FIRST_NEIGHBOURHOOD;
        failures = 0;
        if (onImprove) onImprove(rows.size());
    }
    return rows.size() == target;
}

}  // namespace walk_cover
