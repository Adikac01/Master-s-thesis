// Finds the largest subset of a symbol list that satisfies conditions (A), (B1) and (B2).
// usage: max_subset [file] [output file]
#include "conflicts.hpp"
#include "symbol_io.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

namespace {

const char* const DEFAULT_DATA = "data/chattopadhyay_2022.txt";

}  // namespace

int main(int argc, char* argv[]) {
    const std::string path = (argc > 1) ? argv[1] : DEFAULT_DATA;

    std::ifstream input(path);
    if (!input) {
        std::cerr << "Cannot open file: " << path << "\n"
                  << "Run from the project root, or pass a path explicitly.\n";
        return 2;
    }

    const symbol_io::Table table = symbol_io::read(input);
    if (!table.errors.empty()) {
        for (const symbol_io::ParseError& error : table.errors)
            std::cerr << path << ":" << error.line << ": " << error.message << "\n";
        return 2;
    }
    const conditions::SymbolList symbols = table.symbols();

    const conflicts::ConflictGraph graph = conflicts::build(symbols);
    conditions::SymbolList subset = conflicts::largestAdmissibleSubset(graph);
    std::sort(subset.begin(), subset.end());

    std::cout << "Symbols read: " << symbols.size() << "\n"
              << "Cannot appear in any admissible set:";
    for (const dna::Word& symbol : graph.rejected) std::cout << " " << symbol;
    if (graph.rejected.empty()) std::cout << " none";
    std::cout << "\nPairs in conflict: " << graph.conflictCount() << "\n"
              << "Largest admissible subset: " << subset.size() << " symbols\n";

    if (argc > 2) {
        std::ofstream output(argv[2]);
        if (!output) {
            std::cerr << "Cannot write file: " << argv[2] << "\n";
            return 2;
        }
        output << "# largest admissible subset of " << path << "\n";
        for (const dna::Word& symbol : subset) output << symbol << "\n";
        std::cout << "Written to " << argv[2] << "\n";
    } else {
        for (const dna::Word& symbol : subset) std::cout << symbol << "\n";
    }
    return 0;
}
