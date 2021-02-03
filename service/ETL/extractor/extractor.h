#pragma once

#include <future>
#include <string>
#include <vector>

namespace ETL {
namespace extractor {

class base {
    public:
        virtual std::vector<std::future<std::string>> extract() = 0;
};

} // namespace extractor
} // namespace ETL
