#include "ETL.h"

#include "loader/memory.h"

/*
#include "extractor/moikrug.h"
#include "loader/moikrug.h"
#include "transformer/moikrug.h"
*/

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
                        "vacancy?order_by=publication_time&clusters=true&area="
                        "1&specialization=1.221&enable_snippets=true&page=";

static std::filesystem::path hh_filename{"hh.tsv"};

constexpr auto moikrug_reducer = [](auto xs) {
        std::string left_wrapper{"["};
        std::string right_wrapper{"]"};
        std::string delimeter{", "};

        return std::accumulate(xs.begin() + 1, xs.end(), left_wrapper + xs[0],
                               [&](std::string a, std::string b) {
                                       return std::move(a + delimeter + b);
                               }) +
               right_wrapper;
};

constexpr auto hh_reducer = [](auto xs) {
        /*
        std::string left_wrapper{""};
        std::string right_wrapper{""};
        std::string delimeter{"\n"};

        return std::accumulate(xs.begin() + 1, xs.end(),
                               std::move(left_wrapper + xs[0]),
                               [&](std::string a, std::string b) {
                                       return std::move(a + delimeter + b);
                               }) +
               right_wrapper;
        */

        std::string response_str{};

        response_str += *(xs.begin());

        std::cout << "accumulating responses..." << std::endl;

        for (auto it = xs.begin() + 1; it != xs.end(); it++) {
                response_str += '\n' + *it;
                std::cout << "added response " << (it - xs.begin())
                          << std::endl;
        }

        std::cout << "accumulated responses" << std::endl;

        return std::move(response_str);
};

/*
void spawn_moikrug() {
        ETL::extractor::moikrug moikrug_extractor{};
        ETL::transformer::moikrug moikrug_transformer{};
        ETL::loader::memory memory_loader{};
        ETL::loader::moikrug moikrug_loader{data_dir, moikrug_filename};
        ETL::ETL moikrug{moikrug_extractor, moikrug_transformer, memory_loader};

        auto first_page = moikrug_url + std::to_string(1);

        moikrug_extractor.set_reducer(moikrug_reducer);
        moikrug_extractor.add_url(first_page);

        moikrug.run().get();

        moikrug_extractor.reset_urls();

        auto total_pages = std::stoll(std::string{memory_loader.access()});
        moikrug.set_loader(moikrug_loader);

        size_t i;
        for (i = 1; i <= total_pages; ++i) {
                auto url = moikrug_url + std::to_string(i);
                moikrug_extractor.add_url(url);
        }

        moikrug.run().get();
}
*/

void spawn_hh() {
        ETL::extractor::hh hh_extractor{500};
        ETL::transformer::hh hh_transformer{};
        ETL::loader::memory memory_loader{};
        ETL::loader::hh hh_loader{data_dir, hh_filename};
        ETL::ETL hh{hh_extractor, hh_transformer, memory_loader};

        auto first_page = hh_url + std::to_string(0);

        hh_extractor.set_reducer(hh_reducer);
        hh_extractor.add_url(first_page);

        hh.run().get();
        hh_extractor.reset_urls();

        auto total_pages = std::stoll(std::string{memory_loader.access()});

        size_t i;
        for (i = 0; i < total_pages; ++i) {
                auto url = hh_url + std::to_string(i);
                hh_extractor.add_url(url);
        }

        hh.run().get();

        hh_extractor.reset_urls();

        auto job_urls_str = std::string{memory_loader.access()};
        std::cout << job_urls_str << std::endl;
        auto job_urls = split(job_urls_str, '\n');
        std::cout << job_urls.size() << std::endl;

        for (auto job_url : job_urls)
                hh_extractor.add_url(job_url);

        hh.set_loader(hh_loader);

        hh.run().get();
}

int main() {
        // spawn_moikrug();
        spawn_hh();
}
