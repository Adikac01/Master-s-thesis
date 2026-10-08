#include "symbol_io.hpp"
#include "testing.hpp"

#include <sstream>

namespace {

symbol_io::Table readText(const std::string& text) {
    std::istringstream input(text);
    return symbol_io::read(input);
}

}  // namespace

int main() {
    Suite suite("symbol_io");

    const symbol_io::Table good = readText(
        "# comment\n\n   # indented comment\ngattca  ctaagt\nTCGCTA AGCGAT\n");
    suite.expect("comments and blank lines are skipped", good.rows.size() == 2);
    suite.expect("letters are read case-insensitively",
                 good.errors.empty() && good.rows[0].strand1 == "GATTCA");
    suite.expect("a second column is read as the complementary strand",
                 good.hasComplements() && good.rows[1].strand2 == "AGCGAT");

    const symbol_io::Table single = readText("GATTCA\nTCGCTA\n");
    suite.expect("a single column is fine", single.errors.empty() && !single.hasComplements());

    const symbol_io::Table typo = readText("GATTCA\nTCGCTX\n");
    suite.expect("an invalid symbol is reported with its line",
                 typo.errors.size() == 1 && typo.errors[0].line == 2);

    const symbol_io::Table badStrand = readText("GATTCA CTAAXT\n");
    suite.expect("an invalid complementary strand is reported",
                 badStrand.errors.size() == 1 && badStrand.rows.empty());

    const symbol_io::Table extra = readText("GATTCA CTAAGT EXTRA\n");
    suite.expect("an extra field is reported", extra.errors.size() == 1);

    const symbol_io::Table mixed = readText("GATTCA CTAAGT\nTCGCTA\n");
    suite.expect("a missing strand among complete rows is reported",
                 mixed.errors.size() == 1 && mixed.errors[0].line == 2);

    return suite.finish();
}
