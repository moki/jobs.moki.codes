#pragma once

#include "loader.h"
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string>

namespace ETL {
namespace loader {

class fs : public virtual base {
    public:
        fs(std::filesystem::path &dir, std::filesystem::path &filename);
        std::vector<std::future<std::string>>
        load(std::vector<std::future<std::string>> input) override;

    private:
        std::filesystem::path dir;
        std::filesystem::path filename;
        std::string output;
};

std::vector<std::future<std::string>>
fs::load(std::vector<std::future<std::string>> input) {
        std::cout << "loading" << std::endl;
        std::vector<std::future<std::string>> loaded{};
        loaded.reserve(input.size());

        auto filepath = std::filesystem::path{dir / filename};
        std::filesystem::create_directory(dir);
        std::ofstream file{std::string{filepath}};
        if (!file)
                return std::move(loaded);

        for (auto &future : input) {
                auto tabular = future.get();
                file << tabular;

                loaded.push_back(std::async(std::launch::async, [=]() {
                        return std::move(tabular);
                }));
        }

        file.close();

        return std::move(loaded);
}

fs::fs(std::filesystem::path &dir, std::filesystem::path &filename)
        : dir{dir}, filename{filename} {}

} // namespace loader
} // namespace ETL
