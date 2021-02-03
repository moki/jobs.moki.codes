#pragma once

#include <future>
#include <vector>

namespace ETL {
namespace loader {

class base {
    public:
        virtual std::vector<std::future<std::string>>
        load(std::vector<std::future<std::string>> input) = 0;
};

} // namespace loader
} // namespace ETL
