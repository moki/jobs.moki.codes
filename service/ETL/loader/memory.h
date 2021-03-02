/*
#pragma once

#include "loader.h"
#include <future>
#include <string>

namespace ETL {
namespace loader {

class memory : public virtual base {
    public:
        memory();
        std::future<void> load(std::string_view &&message) override;
        std::string_view access();

    private:
        std::string output;
};

std::future<void> memory::load(std::string_view &&message) {
        return std::async(std::launch::async, [&]() {
                output = std::move(message);
        });
}

std::string_view memory::access() { return output; }

memory::memory() {}

} // namespace loader
} // namespace ETL
*/
