#include "ETL.h"
#include "extractor/moikrug.h"
#include "loader/memory.h"
#include "loader/moikrug.h"
#include "transformer/moikrug.h"
#include <filesystem>
#include <iostream>
#include <string>

using namespace std::string_literals;

constexpr auto moikrug_url =
        "https://career.habr.com/api/frontend/vacancies?q=&page=";

static std::filesystem::path data_dir{std::filesystem::current_path() /
                                      std::filesystem::path{"data"}};

static std::filesystem::path moikrug_filename{"moikrug.tsv"};

void spawn_moikrug() {
        ETL::extractor::moikrug moikrug_extractor{moikrug_url};
        ETL::transformer::moikrug moikrug_transformer{};
        ETL::loader::moikrug moikrug_loader{data_dir, moikrug_filename};
        ETL::ETL moikrug{moikrug_extractor, moikrug_transformer,
                         moikrug_loader};

        moikrug.run().get();

        moikrug_extractor.set_pages_total(
                std::stoll(std::string{moikrug_loader.access()}));

        moikrug.run().get();
}

int main() { spawn_moikrug(); }
