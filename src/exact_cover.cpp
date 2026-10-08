#include "exact_cover.hpp"

#include <limits>
#include <stdexcept>

namespace exact_cover {

namespace {
constexpr int ROOT = 0;
constexpr std::size_t DEADLINE_CHECK_INTERVAL = 4096;  // steps between clock reads
}

Solver::Solver(std::size_t columnCount, std::size_t secondaryCount)
    : columns_(columnCount + secondaryCount) {
    const int primary = static_cast<int>(columnCount);
    const int nodes = static_cast<int>(columns_) + 1;   // root + headers
    left_.resize(nodes);
    right_.resize(nodes);
    up_.resize(nodes);
    down_.resize(nodes);
    column_.resize(nodes);
    rowOf_.resize(nodes, -1);
    size_.assign(nodes, 0);

    for (int i = 0; i < nodes; ++i) {
        left_[i] = right_[i] = i;   // secondary headers stay out of the list
        up_[i] = down_[i] = i;      // empty column
        column_[i] = i;
    }
    for (int i = 0; i <= primary; ++i) {
        left_[i]  = (i == ROOT) ? primary : i - 1;
        right_[i] = (i == primary) ? ROOT : i + 1;
    }
}

void Solver::addRow(const std::vector<std::size_t>& columns) {
    // Check everything first, so a bad row leaves the matrix untouched.
    for (std::size_t i = 0; i < columns.size(); ++i) {
        if (columns[i] >= columns_)
            throw std::out_of_range("exact_cover: column index out of range");
        for (std::size_t j = 0; j < i; ++j)
            if (columns[j] == columns[i])
                throw std::invalid_argument("exact_cover: column repeated in a row");
    }

    int firstOfRow = -1;
    for (const std::size_t requested : columns) {
        const int header = static_cast<int>(requested) + 1;
        const int node = static_cast<int>(left_.size());

        left_.push_back(node);
        right_.push_back(node);
        up_.push_back(up_[header]);
        down_.push_back(header);
        column_.push_back(header);
        rowOf_.push_back(static_cast<int>(rows_));
        size_.push_back(0);

        down_[up_[header]] = node;
        up_[header] = node;
        ++size_[header];

        if (firstOfRow < 0) {
            firstOfRow = node;
        } else {                     // splice into the row's circular list
            left_[node] = left_[firstOfRow];
            right_[node] = firstOfRow;
            right_[left_[firstOfRow]] = node;
            left_[firstOfRow] = node;
        }
    }
    ++rows_;
}

void Solver::cover(int header) {
    right_[left_[header]] = right_[header];
    left_[right_[header]] = left_[header];
    for (int row = down_[header]; row != header; row = down_[row])
        for (int node = right_[row]; node != row; node = right_[node]) {
            up_[down_[node]] = up_[node];
            down_[up_[node]] = down_[node];
            --size_[column_[node]];
        }
}

void Solver::uncover(int header) {
    for (int row = up_[header]; row != header; row = up_[row])
        for (int node = left_[row]; node != row; node = left_[node]) {
            ++size_[column_[node]];
            up_[down_[node]] = node;
            down_[up_[node]] = node;
        }
    right_[left_[header]] = header;
    left_[right_[header]] = header;
}

int Solver::smallestColumn() const {
    int best = -1;
    int bestSize = std::numeric_limits<int>::max();
    for (int header = right_[ROOT]; header != ROOT; header = right_[header])
        if (size_[header] < bestSize) {
            bestSize = size_[header];
            best = header;
        }
    return best;
}

void Solver::search() {
    if (stopped_) return;
    if (hasDeadline_ && steps_ % DEADLINE_CHECK_INTERVAL == 0 &&
        std::chrono::steady_clock::now() >= deadline_) {
        stopped_ = true;
        timedOut_ = true;
        return;
    }
    if (right_[ROOT] == ROOT) {              // every column covered
        if (!onSolution_(partial_)) stopped_ = true;
        return;
    }

    const int header = smallestColumn();
    if (size_[header] == 0) return;          // dead end

    cover(header);
    for (int row = down_[header]; row != header && !stopped_; row = down_[row]) {
        ++steps_;
        partial_.push_back(static_cast<std::size_t>(rowOf_[row]));
        if (partial_.size() > deepest_.size()) deepest_ = partial_;

        for (int node = right_[row]; node != row; node = right_[node])
            cover(column_[node]);

        search();

        // Undo in reverse order so the matrix returns to its previous state.
        for (int node = left_[row]; node != row; node = left_[node])
            uncover(column_[node]);
        partial_.pop_back();
    }
    uncover(header);
}

void Solver::setDeadline(std::chrono::steady_clock::time_point deadline) {
    deadline_ = deadline;
    hasDeadline_ = true;
}

void Solver::enumerate(const std::function<bool(const Row&)>& onSolution) {
    onSolution_ = onSolution;
    stopped_ = false;
    timedOut_ = false;
    partial_.clear();
    deepest_.clear();
    steps_ = 0;
    search();
    onSolution_ = nullptr;  // it may refer to the caller's locals
}

bool Solver::solve(Row& solution) {
    bool found = false;
    enumerate([&](const Row& candidate) {
        solution = candidate;
        found = true;
        return false;                        // stop at the first one
    });
    return found;
}

}  // namespace exact_cover
