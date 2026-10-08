#ifndef TESTING_HPP
#define TESTING_HPP

#include <iostream>
#include <string>

class Suite {
public:
    explicit Suite(const std::string& name) { std::cout << name << "\n"; }

    void expect(const std::string& description, bool passed) {
        std::cout << (passed ? "  [PASS] " : "  [FAIL] ") << description << "\n";
        if (!passed) ++failures_;
    }

    int finish() const {
        if (failures_ > 0) std::cout << "  " << failures_ << " failed\n";
        std::cout << "\n";
        return failures_ == 0 ? 0 : 1;
    }

private:
    int failures_ = 0;
};

#endif
