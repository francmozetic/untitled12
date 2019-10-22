#include <algorithm>
#include <functional>
#include <iostream>

// function object
class GreaterLength {
    public:
        bool operator()(const std::string& f, const std::string& s) const {
            return f.length() > s.length();
    }
};
