#pragma once

#include <future>
#include <string>

namespace ETL {
namespace transformer {

class base {
    public:
        virtual std::future<std::string>
        transform(std::string_view &&message) = 0;
};

} // namespace transformer
} // namespace ETL
