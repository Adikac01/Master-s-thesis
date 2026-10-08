// Structure of B(4,4) and of the subgraph spanned by a solution.
// usage: graph_analysis [file | --table]
#include "conditions.hpp"
#include "debruijn.hpp"
#include "dna.hpp"
#include "report.hpp"
#include "symbol_io.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {

void describe(const debruijn::Graph& graph) {
    std::cout << "B(" << graph.alphabet().size() << "," << graph.edgeLength()
              << ")\n"
              << "  vertices (words of length " << graph.vertexLength()
              << "): " << graph.vertexCount() << "\n"
              << "  edges (words of length " << graph.edgeLength()
              << "): " << graph.edgeCount() << "\n"
              << "  in- and out-degree of every vertex: "
              << graph.alphabet().size() << "\n\n"
              << "Involution rho on edges\n"
              << "  two-element orbits: " << graph.edgeOrbitCount() << "\n"
              << "  fixed points (self-complementary): "
              << graph.selfComplementaryEdgeCount() << "\n\n"
              << "Involution rho on vertices\n"
              << "  mirror pairs: " << graph.vertexPairCount() << "\n"
              << "  fixed points: " << graph.selfComplementaryVertexCount()
              << "\n\n"
              << "Lower bound on the imbalance of any transversal: "
              << graph.imbalanceLowerBound() << "\n";
    if (graph.imbalanceLowerBound() > 0)
        std::cout << "  (so no transversal is balanced, and none admits an "
                     "Eulerian circuit)\n";
    std::cout << "\n";
}

bool analyseSolution(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        std::cerr << "Cannot open file: " << path << "\n";
        return false;
    }
    const symbol_io::Table table = symbol_io::read(input);
    if (!table.errors.empty()) {
        for (const symbol_io::ParseError& error : table.errors)
            std::cerr << path << ":" << error.line << ": " << error.message << "\n";
        return false;
    }
    const conditions::SymbolList symbols = table.symbols();
    if (symbols.empty()) {
        std::cerr << "No symbols found in " << path << "\n";
        return false;
    }

    std::set<dna::Word> distinct;
    for (const dna::Word& symbol : symbols)
        for (const dna::Word& edge : dna::symbolEdges(symbol)) distinct.insert(edge);
    const std::vector<dna::Word> edges(distinct.begin(), distinct.end());
    const debruijn::SubgraphStats stats = debruijn::analyseSubgraph(edges);

    std::cout << "Subgraph spanned by " << path << "\n"
              << "  symbols: " << symbols.size()
              << "   edges: " << edges.size()
              << "   vertices touched: " << stats.vertices << "\n"
              << "  unbalanced vertices: " << stats.unbalanced
              << "   total imbalance: " << stats.imbalance << "\n"
              << "  weakly connected: " << (stats.weaklyConnected ? "yes" : "no")
              << "\n\n";
    return true;
}

void printFamilyTable() {
    std::cout << "  k    n   vertices     edges   orbits  fixed edges  "
                 "mirror pairs  fixed vertices  imbalance bound\n";
    for (std::size_t k = 2; k <= 4; k += 2)
        for (std::size_t n = 2; n <= 8; ++n) {
            const debruijn::Graph graph(k, n);
            std::cout << std::setw(3) << k << std::setw(5) << n
                      << std::setw(11) << graph.vertexCount()
                      << std::setw(10) << graph.edgeCount()
                      << std::setw(9) << graph.edgeOrbitCount()
                      << std::setw(13) << graph.selfComplementaryEdgeCount()
                      << std::setw(14) << graph.vertexPairCount()
                      << std::setw(16) << graph.selfComplementaryVertexCount()
                      << std::setw(17) << graph.imbalanceLowerBound() << "\n";
        }
    std::cout << "\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string argument = (argc > 1) ? argv[1] : std::string();

    if (argument == "--table") {
        report::heading("The family of de Bruijn graphs B(k,n)");
        printFamilyTable();
        return 0;
    }

    report::heading("Structure of the graph underlying the problem");
    describe(debruijn::Graph(dna::ALPHABET_SIZE, dna::EDGE_LENGTH));
    if (!argument.empty() && !analyseSolution(argument)) return 2;
    return 0;
}
