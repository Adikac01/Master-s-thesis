#include "symbol_io.hpp"

#include <cctype>
#include <sstream>

namespace symbol_io {

dna::Word normalise(const std::string& field) {
    dna::Word word;
    word.reserve(field.size());
    for (const char character : field) {
        const char upper =
            static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        if (upper != 'A' && upper != 'C' && upper != 'G' && upper != 'T')
            return dna::Word();
        word += upper;
    }
    return word;
}

Table read(std::istream& input) {
    Table table;
    std::vector<std::size_t> lineOf;
    std::string line;
    for (std::size_t number = 1; std::getline(input, line); ++number) {
        std::istringstream fields(line);
        std::string first, second, extra;
        if (!(fields >> first) || first[0] == '#') continue;

        const dna::Word strand1 = normalise(first);
        if (strand1.empty()) {
            table.errors.push_back({number, "invalid symbol '" + first + "'"});
            continue;
        }
        dna::Word strand2;
        if (fields >> second) {
            strand2 = normalise(second);
            if (strand2.empty()) {
                table.errors.push_back(
                    {number, "invalid complementary strand '" + second + "'"});
                continue;
            }
        }
        if (fields >> extra) {
            table.errors.push_back({number, "unexpected field '" + extra + "'"});
            continue;
        }
        table.rows.push_back({strand1, strand2});
        lineOf.push_back(number);
    }

    // A second column in some rows but not in others is almost certainly a mistake.
    std::size_t withStrand = 0;
    for (const conditions::StrandPair& row : table.rows)
        if (!row.strand2.empty()) ++withStrand;
    if (withStrand > 0 && withStrand < table.rows.size())
        for (std::size_t i = 0; i < table.rows.size(); ++i)
            if (table.rows[i].strand2.empty())
                table.errors.push_back(
                    {lineOf[i], "complementary strand missing (other rows have one)"});
    return table;
}

conditions::SymbolList Table::symbols() const {
    conditions::SymbolList result;
    result.reserve(rows.size());
    for (const conditions::StrandPair& row : rows) result.push_back(row.strand1);
    return result;
}

bool Table::hasComplements() const {
    if (rows.empty()) return false;
    for (const conditions::StrandPair& row : rows)
        if (row.strand2.empty()) return false;
    return true;
}

}  // namespace symbol_io
