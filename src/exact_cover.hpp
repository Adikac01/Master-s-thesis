#ifndef EXACT_COVER_HPP
#define EXACT_COVER_HPP

// Knuth's Algorithm X with dancing links.

#include <chrono>
#include <cstddef>
#include <functional>
#include <vector>

namespace exact_cover {

using Row = std::vector<std::size_t>;

class Solver {
public:
    // Secondary columns come after the primary ones: a cover may leave them
    // uncovered, but never covers one twice.
    explicit Solver(std::size_t columnCount, std::size_t secondaryCount = 0);

    // Columns are 0-based; rows are numbered in the order they are added.
    // Throws if a column is out of range or repeated within the row.
    void addRow(const std::vector<std::size_t>& columns);

    // The callback gets every cover found; returning false stops the search.
    void enumerate(const std::function<bool(const Row&)>& onSolution);
    bool solve(Row& solution);

    // After the deadline the search stops; timedOut() tells whether it did.
    void setDeadline(std::chrono::steady_clock::time_point deadline);
    bool timedOut() const { return timedOut_; }

    // The largest partial cover reached by the last search.
    const Row& deepest() const { return deepest_; }

private:
    void cover(int header);
    void uncover(int header);
    int smallestColumn() const;
    void search();

    std::vector<int> left_, right_, up_, down_, column_, rowOf_, size_;
    std::size_t columns_ = 0;
    std::size_t rows_ = 0;
    std::size_t steps_ = 0;

    Row partial_;
    Row deepest_;
    bool stopped_ = false;
    bool hasDeadline_ = false;
    bool timedOut_ = false;
    std::chrono::steady_clock::time_point deadline_{};
    std::function<bool(const Row&)> onSolution_;
};

}  // namespace exact_cover

#endif
