// Checks a list of symbols against the conditions from chapter 2.
// usage: verifier [file]
#include "conditions.hpp"
#include "dna.hpp"
#include "junctions.hpp"
#include "report.hpp"
#include "symbol_io.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

const char* const DEFAULT_DATA = "data/waldmajer_sakowski_2019.txt";

void printGraphFacts() {
    std::cout << "Graph B(4,4): "
              << dna::allWords(dna::VERTEX_LENGTH).size() << " vertices, "
              << dna::allWords(dna::EDGE_LENGTH).size() << " edges\n"
              << "Involution rho: " << dna::countOrbits(dna::EDGE_LENGTH)
              << " two-element orbits, "
              << dna::countFixedPoints(dna::EDGE_LENGTH) << " fixed points\n\n";
}

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

    report::heading("Verification of symbol list: " + path);
    printGraphFacts();
    // The choice of banned sides (section 3.4) under which this set is counted.
    const std::vector<junctions::Split> splits = junctions::allSplits();
    unsigned choice = 0;
    for (const dna::Word& symbol : symbols) choice |= junctions::suffixMask(symbol, splits);
    std::cout << "Symbols read: " << symbols.size() << "\n"
              << "Choice of banned sides determined by the set: " << choice << "\n\n";
    if (symbols.empty()) {
        std::cerr << "No symbols found in file.\n";
        return 2;
    }

    std::vector<conditions::CheckResult> results = conditions::verifyAll(symbols);
    if (table.hasComplements())
        results.push_back(conditions::strandsComplementary(table.rows));

    return report::summary(results) == 0 ? 0 : 1;
}
