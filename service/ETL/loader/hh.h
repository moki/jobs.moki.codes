#pragma once

#include "fs.h"
#include <future>
#include <string>

namespace ETL {
namespace loader {

class hh : public fs {
    public:
        using fs::fs;

        hh(std::filesystem::path &dir, std::filesystem::path &filename);
};

hh::hh(std::filesystem::path &dir, std::filesystem::path &filename)
        : fs{dir, filename} {}

} // namespace loader
} // namespace ETL
