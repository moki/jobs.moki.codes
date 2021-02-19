#include "extractor/moikrug.h"
#include "loader/moikrug.h"
#include "transformer/moikrug.h"

#include "extractor/telegram.h"
#include "loader/telegram.h"
#include "transformer/profunctor.h"

#include "extractor/hh.h"
#include "loader/hh.h"
#include "transformer/hh.h"

#include "../common/utility/split.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <streambuf>
#include <string>

#include "lib/date/main.h"

using namespace std::string_literals;
using namespace std::chrono;
using namespace date;

static std::filesystem::path data_dir{std::filesystem::current_path() /
                                      std::filesystem::path{"data"}};

constexpr auto moikrug_url =
        "https://career.habr.com/api/frontend/vacancies?q=&page=";

static std::filesystem::path moikrug_filename{"moikrug.tsv"};

constexpr auto profunctor_url = "https://t.me/s/profunctor_jobs?before=";

static std::filesystem::path profunctor_filename{"profunctor.tsv"};

static std::filesystem::path hh_filename{"hh.tsv"};

constexpr auto hh_url = "https://api.hh.ru/vacancies"
                        "?specialization=1.221&specialization=1.25"
                        "&per_page=100";

constexpr auto hh_query_from = "&date_from=";
constexpr auto hh_query_to = "&date_to=";
constexpr auto hh_query_page = "&page=";

constexpr auto hh_date_format = "%FT%T";

constexpr auto moscow_timezone = "Europe/Moscow";

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

void spawn_telegram() {
        ETL::extractor::telegram extractor{25};
        ETL::transformer::profunctor transformer{};
        ETL::loader::telegram loader{data_dir, profunctor_filename};

        auto first_page = std::string{profunctor_url};
        extractor.add_url(first_page);

        auto meta_future = transformer.transform(extractor.extract());

        auto meta = split(meta_future.back().get(), ' ');
        if (meta.size() != 2)
                std::cerr << "failed to parse meta" << std::endl;

        int64_t it = std::stoll(meta[0]);
        int64_t step = std::stoll(meta[1]) + 1;

        extractor.reset_urls();

        first_page = std::string{profunctor_url};

        extractor.add_url(first_page);

        auto stop = 0;

        for (; it > stop; it -= step) {
                auto url = profunctor_url + std::to_string(it);

                extractor.add_url(url);
        }

        auto loaded = loader.load(transformer.transform(extractor.extract()));
        if (!loaded.size()) {
                std::cerr << "faled to load profunctor" << std::endl;
                exit(1);
        }

        for (auto &future : loaded)
                std::cout << future.get();
}

void spawn_hh() {
        auto now_time = system_clock::now();

        auto span_time = hours(24);

        auto step_time = hours(1);

        auto cursor_time = now_time - span_time;

        auto timezone = locate_zone(moscow_timezone);

        constexpr auto user_agent = "jobs.moki.codes";

        ETL::extractor::hh extractor{user_agent, 15};
        ETL::transformer::hh transformer{};
        ETL::loader::hh loader{data_dir, hh_filename};

        std::vector<std::string> hourly_urls;

        for (; cursor_time < now_time;) {
                auto zoned_from =
                        make_zoned(timezone, floor<minutes>(cursor_time));

                auto next_time = cursor_time + step_time;

                auto zoned_to = make_zoned(timezone, floor<minutes>(next_time));

                auto from_time =
                        std::string{format(hh_date_format, zoned_from)};

                auto to_time = std::string{format(hh_date_format, zoned_to)};

                auto url = hh_url + std::string{hh_query_from} + from_time +
                           std::string{hh_query_to} + to_time;

                hourly_urls.push_back(std::string{url});

                extractor.add_url(url);

                cursor_time = next_time;
        }

        auto transformed = transformer.transform(extractor.extract());

        size_t i = 0;

        extractor.reset_urls();

        for (auto &future : transformed) {
                auto pages = std::stoll(future.get());

                auto url = std::move(hourly_urls[i]);

                for (size_t j = 0; j < pages; j++) {
                        auto link = url + hh_query_page + std::to_string(j);

                        extractor.add_url(link);
                }

                i++;
        }

        auto urls = transformer.transform(extractor.extract());

        std::set<std::string> unique_urls{};

        for (auto &future : urls)
                unique_urls.insert(std::move(future.get()));

        extractor.reset_urls();

        for (auto url : unique_urls)
                extractor.add_url(url);

        auto jobs = loader.load(transformer.transform(extractor.extract()));
        if (!jobs.size()) {
                std::cerr << "faled to load profunctor" << std::endl;
                exit(1);
        }
}

int main() {
        spawn_moikrug();
        spawn_telegram();
        spawn_hh();
}
