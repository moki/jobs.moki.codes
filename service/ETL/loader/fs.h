#pragma once

#include "loader.h"
#include <filesystem>
#include <fstream>
#include <future>
#include <string>

namespace ETL {
namespace loader {

class fs : public virtual base {
    public:
        fs(std::filesystem::path &dir, std::filesystem::path &filename);
        std::future<void> load(std::string_view &&message) override;
        std::string_view access();

    private:
        std::filesystem::path dir;
        std::filesystem::path filename;
        std::string output;
};

std::future<void> fs::load(std::string_view &&message) {
        return std::async(std::launch::async, [&]() {
                auto filepath = std::filesystem::path{dir / filename};

                output = std::move(message);

                std::filesystem::create_directory(dir);
                std::ofstream file{std::string{filepath}};

                file << output;
                file.close();
        });
}

std::string_view fs::access() { return output; }

fs::fs(std::filesystem::path &dir, std::filesystem::path &filename)
        : dir{dir}, filename{filename} {}

} // namespace loader
} // namespace ETL
