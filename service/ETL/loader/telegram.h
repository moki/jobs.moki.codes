#pragma once

#include "fs.h"
#include <future>
#include <string>

namespace ETL {
namespace loader {

class telegram : public fs {
    public:
        using fs::fs;

        telegram(std::filesystem::path &dir, std::filesystem::path &filename);
};

telegram::telegram(std::filesystem::path &dir, std::filesystem::path &filename)
        : fs{dir, filename} {}

} // namespace loader
} // namespace ETL
