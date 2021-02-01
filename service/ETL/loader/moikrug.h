#pragma once

#include "fs.h"
#include <future>
#include <string>

namespace ETL {
namespace loader {

class moikrug : public fs {
    public:
        using fs::fs;

        moikrug(std::filesystem::path &dir, std::filesystem::path &filename);
};

moikrug::moikrug(std::filesystem::path &dir, std::filesystem::path &filename)
        : fs{dir, filename} {}

} // namespace loader
} // namespace ETL
