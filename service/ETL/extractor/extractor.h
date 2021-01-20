#pragma once

#include <future>
#include <string>

namespace ETL {
namespace extractor {

class base {
    public:
        virtual std::future<std::string> extract() = 0;
};

} // namespace extractor
} // namespace ETL
