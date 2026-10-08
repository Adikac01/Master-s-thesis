#ifndef REPORT_HPP
#define REPORT_HPP

#include "conditions.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace report {

inline void heading(const std::string& title) {
    std::cout << "=====================================================\n"
              << " " << title << "\n"
              << "=====================================================\n\n";
}

inline void line(const conditions::CheckResult& result) {
    std::cout << (result.passed ? "  [PASS] " : "  [FAIL] ") << result.name;
    if (!result.detail.empty()) std::cout << "  --> " << result.detail;
    std::cout << "\n";
}

// Prints every result and returns the number of failures.
inline std::size_t summary(const std::vector<conditions::CheckResult>& results) {
    std::size_t failures = 0;
    for (const conditions::CheckResult& result : results) {
        line(result);
        if (!result.passed) ++failures;
    }
    std::cout << "\n-----------------------------------------------------\n";
    if (failures == 0)
        std::cout << " RESULT: all checks passed.\n";
    else
        std::cout << " RESULT: failed checks: " << failures << "\n";
    std::cout << "-----------------------------------------------------\n";
    return failures;
}

}  // namespace report

#endif
