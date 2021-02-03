#include "extractor/moikrug.h"
#include "loader/moikrug.h"
#include "transformer/moikrug.h"

#include "extractor/hh.h"
#include "loader/hh.h"
#include "transformer/hh.h"

#include "../common/utility/split.h"

#include <filesystem>
#include <iostream>
#include <string>

#include <fstream>
#include <streambuf>

using namespace std::string_literals;

constexpr auto moikrug_url =
        "https://career.habr.com/api/frontend/vacancies?q=&page=";

static std::filesystem::path data_dir{std::filesystem::current_path() /
                                      std::filesystem::path{"data"}};

static std::filesystem::path moikrug_filename{"moikrug.tsv"};

constexpr auto hh_url = "https://hh.ru/search/"
                        "vacancy"
                        //"?L_is_autosearch=false"
                        "?order_by=publication_time"
                        "&clusters=true"
                        "&area="
                        "1&specialization=1.221"
                        //"&enable_snippets=true"
                        "&enable_snippets=true"
                        "&items_on_page=25"
                        //"&no_magic=true"
                        "&page=";

static std::filesystem::path hh_filename{"hh.tsv"};

void spawn_moikrug() {
        ETL::extractor::moikrug extractor{50};
        ETL::transformer::moikrug transformer{};
        ETL::loader::moikrug loader{data_dir, moikrug_filename};

        auto url = moikrug_url + std::to_string(1);
        extractor.add_url(url);

        auto pages = std::stoll(
                transformer.transform(extractor.extract()).back().get());

        extractor.reset_urls();

        for (size_t i = 1; i <= pages; ++i) {
                auto url = moikrug_url + std::to_string(i);
                extractor.add_url(url);
        }

        auto loaded = loader.load(transformer.transform(extractor.extract()));
        if (!loaded.size()) {
                std::cerr << "failed to load moikrug" << std::endl;
                exit(1);
        }
}

void spawn_hh() {
        ETL::extractor::hh extractor{100};
        ETL::transformer::hh transformer{};
        ETL::loader::hh loader{data_dir, hh_filename};

        auto url = hh_url + std::to_string(0);
        extractor.add_url(url);

        auto pages = std::stoll(
                transformer.transform(extractor.extract()).back().get());

        extractor.reset_urls();

        for (size_t i = 0; i < pages; ++i) {
                auto url = hh_url + std::to_string(i);
                extractor.add_url(url);
        }

        auto urls = transformer.transform(extractor.extract());

        extractor.reset_urls();

        for (auto &future : urls) {
                auto url = future.get();
                extractor.add_url(url);
        }

        auto loaded = loader.load(transformer.transform(extractor.extract()));
        if (!loaded.size()) {
                std::cerr << "failed to load hh" << std::endl;
                exit(1);
        }
}

int main() {
        spawn_moikrug();
        spawn_hh();
}
