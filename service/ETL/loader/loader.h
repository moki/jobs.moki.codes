#pragma once

#include <future>

namespace ETL {
namespace loader {

class base {
    public:
        virtual std::future<void> load(std::string_view &&message) = 0;
};

} // namespace loader
} // namespace ETL
