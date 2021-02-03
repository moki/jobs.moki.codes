#pragma once

#include <future>
#include <string>
#include <vector>

namespace ETL {
namespace transformer {

class base {
    public:
        // virtual std::future<std::string>
        // transform(std::string_view &&message) = 0;
        virtual std::vector<std::future<std::string>>
        transform(std::vector<std::future<std::string>> input) = 0;
};

} // namespace transformer
} // namespace ETL
