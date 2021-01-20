#pragma once

#include <future>
#include <string>

namespace ETL {
namespace transformer {

class base {
    public:
        std::future<std::string> transform();
};

} // namespace transformer
} // namespace ETL
