#ifndef SYMBOL_IO_HPP
#define SYMBOL_IO_HPP

#include "conditions.hpp"

#include <cstddef>
#include <istream>
#include <string>
#include <vector>

namespace symbol_io {

struct ParseError {
    std::size_t line;
    std::string message;
};

struct Table {
    std::vector<conditions::StrandPair> rows;
    std::vector<ParseError> errors;

    conditions::SymbolList symbols() const;
    bool hasComplements() const;  // every row has a second strand
};

// One symbol per line; blank lines and lines starting with '#' are skipped.
// An optional second column holds the complementary strand. Lines that
// cannot be read end up in errors instead of being dropped.
Table read(std::istream& input);

// Returns an empty word if the field contains anything other than ACGT.
dna::Word normalise(const std::string& field);

}  // namespace symbol_io

#endif
