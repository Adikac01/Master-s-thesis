// =====================================================================
//  table4.hpp
//  The 40 symbols published in Table 4 of Waldmajer & Sakowski (2019),
//  Informatica 43 (2019) 485-494, p. 493.
// =====================================================================
#ifndef TABLE4_HPP
#define TABLE4_HPP

#include "conditions.hpp"

#include <vector>

namespace table4 {

// Strand 1 (5'-3') together with strand 2 (3'-5'), as printed.
const std::vector<conditions::StrandPair>& rows();

// Strand 1 of every row: the symbols themselves.
conditions::SymbolList symbols();

} // namespace table4

#endif  // TABLE4_HPP
