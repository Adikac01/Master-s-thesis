// Searches B(4,n) for walks of length 3 that use every rho-orbit exactly once
// (section 4.5), or checks a saved family.
// usage: walk_family [n] [seconds] [seed]
//        walk_family --verify file [n]
#include "debruijn.hpp"
#include "report.hpp"
#include "walk_cover.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

const double MAX_RESTART_PHASE = 60.0;  // seconds; the rest goes to repairing
const double RESTART_SLICE = 0.1;

std::string outputPath(std::size_t n) {
    return "data/walk_cover_n" + std::to_string(n) + ".txt";
}

std::vector<debruijn::Word> readWalks(std::istream& input) {
    std::vector<debruijn::Word> walks;
    std::string line;
    while (std::getline(input, line))
        if (!line.empty() && line[0] != '#') walks.push_back(line);
    return walks;
}

int verify(const std::string& path, std::size_t n) {
    std::ifstream input(path);
    if (!input) {
        std::cerr << "Cannot open file: " << path << "\n";
        return 2;
    }
    const debruijn::Alphabet dna("ACGT");
    const std::vector<debruijn::Word> walks = readWalks(input);
    const bool ok = walk_cover::usesEveryOrbitOnce(dna, n, walks);
    std::cout << path << ": " << walks.size() << " walks, "
              << (ok ? "every orbit of B(4," + std::to_string(n) + ") used exactly once\n"
                     : "NOT a family using every orbit exactly once\n");
    return ok ? 0 : 1;
}

int search(std::size_t n, double seconds, unsigned seed) {
    using walk_cover::Clock;
    const auto started = Clock::now();
    const auto after = [&](double s) {
        return started + std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(s));
    };
    const auto elapsed = [&] {
        return std::chrono::duration<double>(Clock::now() - started).count();
    };

    report::heading("Walks of length 3 using every orbit of B(4," + std::to_string(n) + ")");
    const debruijn::Alphabet dna("ACGT");
    const walk_cover::Instance instance = walk_cover::build(dna, n);
    const std::size_t target = instance.columnCount / 3;
    std::cout << "Orbits: " << instance.columnCount << "   walks to choose: " << target
              << "   candidate walks: " << instance.walks.size() << "\n";
    if (instance.columnCount % 3 != 0) {
        std::cout << "The number of orbits is not divisible by 3, so no such family exists.\n";
        return 1;
    }

    std::mt19937 rng(seed);
    const double restartPhase = std::min(MAX_RESTART_PHASE, seconds / 4);
    std::vector<std::size_t> rows =
        walk_cover::randomRestarts(instance, rng, RESTART_SLICE, after(restartPhase));
    std::cout << "Random restarts: " << rows.size() << " of " << target << " walks\n";

    const bool complete = walk_cover::repair(
        instance, rows, rng, after(seconds), [&](std::size_t walks) {
            std::cout << "  repaired to " << walks << " walks after " << elapsed() << " s"
                      << std::endl;
        });

    std::vector<debruijn::Word> words;
    for (const std::size_t row : rows) words.push_back(instance.walks[row]);
    std::sort(words.begin(), words.end());
    if (!complete) {
        std::cout << "\nNo complete family found; best: " << rows.size() << " of " << target
                  << " walks.\n";
        return 1;
    }
    if (!walk_cover::usesEveryOrbitOnce(dna, n, words)) {
        std::cout << "\nInternal error: the family failed the independent check.\n";
        return 1;
    }

    const std::string path = outputPath(n);
    std::ofstream output(path);
    output << "# " << words.size() << " walks of length 3 in B(4," << n
           << ") using every rho-orbit exactly once.\n"
           << "# Found by bin/walk_family (seed " << seed << "); check with\n"
           << "# bin/walk_family --verify " << path << " " << n << "\n";
    for (const debruijn::Word& word : words) output << word << "\n";
    std::cout << "\nFound a complete family of " << words.size() << " walks after " << elapsed()
              << " s; it passes the independent check.\nWritten to " << path << "\n";
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    try {
        if (!args.empty() && args[0] == "--verify") {
            if (args.size() < 2 || args.size() > 3) {
                std::cerr << "usage: walk_family --verify file [n]\n";
                return 2;
            }
            return verify(args[1], args.size() == 3 ? std::stoul(args[2]) : 6);
        }
        if (args.size() > 3) {
            std::cerr << "usage: walk_family [n] [seconds] [seed]\n";
            return 2;
        }
        const std::size_t n = args.size() > 0 ? std::stoul(args[0]) : 6;
        const double seconds = args.size() > 1 ? std::stod(args[1]) : 600.0;
        const unsigned seed = args.size() > 2 ? static_cast<unsigned>(std::stoul(args[2])) : 1;
        if (n < 2 || n > 8 || seconds <= 0) {
            std::cerr << "n must be between 2 and 8 and seconds positive\n";
            return 2;
        }
        return search(n, seconds, seed);
    } catch (const std::exception&) {
        std::cerr << "usage: walk_family [n] [seconds] [seed]\n"
                     "       walk_family --verify file [n]\n";
        return 2;
    }
}
