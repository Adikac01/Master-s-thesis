#include "table4.hpp"

namespace table4 {

const std::vector<conditions::StrandPair>& rows() {
    static const std::vector<conditions::StrandPair> published = {
        {"TCGCTA","AGCGAT"}, {"CGTTCG","GCAAGC"}, {"ATTGAT","TAACTA"},
        {"CGAGTA","GCTCAT"}, {"CAGGGG","GTCCCC"}, {"TAGATA","ATCTAT"},
        {"ATAGTT","TATCAA"}, {"GTTTTG","CAAAAC"}, {"TTGTTG","AACAAC"},
        {"TTGGTG","AACCAC"}, {"GTGCCG","CACGGC"}, {"CTAATG","GATTAC"},
        {"ATGCGT","TACGCA"}, {"CGTGAG","GCACTC"}, {"GAGCAA","CTCGTT"},
        {"CAAGCC","GTTCGG"}, {"GCCTTT","CGGAAA"}, {"TTTCTG","AAAGAC"},
        {"CTGAAT","GACTTA"}, {"AATCCC","TTAGGG"}, {"GCCCGC","CGGGCG"},
        {"CGCCAG","GCGGTC"}, {"ATGGGT","TACCCA"}, {"GGTAGG","CCATCC"},
        {"AGGTTA","TCCAAT"}, {"GCTGTG","CGACAC"}, {"GTGTAT","CACATA"},
        {"TATTTA","ATAAAT"}, {"TTACGA","AATGCT"}, {"CGACTT","GCTGAA"},
        {"CTTCCG","GAAGGC"}, {"CCGTCT","GGCAGA"}, {"TCTTAT","AGAATA"},
        {"TATGTC","ATACAG"}, {"GTCATC","CAGTAG"}, {"ATCGGT","TAGCCA"},
        {"GGTCCT","CCAGGA"}, {"CCTCTC","GGAGAG"}, {"CTCCAC","GAGGTG"},
        {"CACTGC","GTGACG"}
    };
    return published;
}

conditions::SymbolList symbols() {
    conditions::SymbolList result;
    result.reserve(rows().size());
    for (const conditions::StrandPair& row : rows())
        result.push_back(row.strand1);
    return result;
}

} // namespace table4
