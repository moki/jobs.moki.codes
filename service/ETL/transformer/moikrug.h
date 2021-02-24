#pragma once

#include "transformer.h"
#include <numeric>
#include <string>

#include "../lib/simdjson/main.h"

#include "../../common/utility/trim.h"

using namespace std::string_literals;

namespace ETL {
namespace transformer {

class moikrug : public base {
        static inline std::string nullstr = "\\N"s;

    public:
        enum class mode { meta, job };

        moikrug();
        std::vector<std::future<std::string>>
        transform(std::vector<std::future<std::string>> input);

    private:
        simdjson::dom::parser parser;
        mode parse_mode;

        std::string parse_pages_total(auto &meta);
        std::string parse_job(auto &);
        std::string parse_skills(auto &);
        std::string parse_salary(auto &);
        std::string_view parse_date(auto &);
        std::string parse_locations(auto &);
};

std::string moikrug::parse_pages_total(auto &meta) {
        return std::move(
                std::to_string(static_cast<size_t>(meta["totalPages"])));
}

std::string_view moikrug::parse_date(auto &date) {
        auto i = date.find("T");
        if (i == std::string::npos)
                exit(1);

        return date.substr(0, i);
}

std::string moikrug::parse_skills(auto &skills) {
        std::vector<std::string> skills_vec;

        for (simdjson::dom::element skill_element : skills) {
                simdjson::dom::object skill;
                auto err = skill_element.get(skill);
                if (err)
                        break;

                std::string_view skill_str;

                err = skill["title"].get(skill_str);
                skills_vec.push_back("'" + std::string{skill_str} + "'");
        }

        if (!skills_vec.size())
                return "[]"s;

        auto skills_res =
                std::accumulate(skills_vec.begin() + 1, skills_vec.end(),
                                std::move("["s + *(skills_vec.begin())),
                                [](std::string a, std::string b) {
                                        return std::move(a) + std::move(",") +
                                               std::move(b);
                                }) +
                "]"s;

        return std::move(skills_res);
}

std::string moikrug::parse_salary(auto &salary) {
        size_t from, to;
        std::string range{"["};
        std::string_view currency;

        uint8_t state = 0;

        auto err = salary["currency"].get(currency);
        if (err)
                return nullstr + "\t"s + "[]"s;

        err = salary["from"].get(from);
        if (!err)
                state++;

        err = salary["to"].get(to);
        if (!err)
                state++;

        switch (state) {
        case 1: {
                auto single =
                        salary["from"].type() ==
                                        simdjson::dom::element_type::NULL_VALUE
                                ? to
                                : from;
                range += std::to_string(single) + "," + std::to_string(single) +
                         "]"s;
                break;
        }
        case 2: {
                range += std::to_string(from) + "," + std::to_string(to) + "]"s;
                break;
        }
        default: {
                range = "[]"s;
                break;
        }
        }

        return std::move(std::string{currency} + "\t"s + range);
}

std::string moikrug::parse_locations(auto &locations) {
        std::vector<std::string> titles;

        for (simdjson::dom::element location_element : locations) {
                simdjson::dom::object location;
                std::string_view title;

                auto err = location_element.get(location);
                if (err)
                        break;

                err = location["title"].get(title);
                titles.push_back("'" + std::string{title} + "'");
        }

        if (!titles.size())
                return "[]"s;

        return std::accumulate(titles.begin() + 1, titles.end(),
                               std::move("["s + *(titles.begin())),
                               [](std::string a, std::string b) {
                                       return std::move(a + "," + b);
                               }) +
               "]"s;
}

std::string moikrug::parse_job(auto &job) {
        std::string parsed{};

        std::string_view title;
        std::string_view date;
        simdjson::dom::array locations;
        simdjson::dom::element salary;
        simdjson::dom::array skills;
        bool remote;
        size_t id;

        auto err = job["id"].get(id);
        parsed += (err ? nullstr : std::to_string(id)) + "\t"s;

        err = job["publishedDate"]["date"].get(date);
        parsed += (err ? nullstr : std::string{parse_date(date)}) + "\t"s;

        err = job["title"].get(title);
        std::string title_str{title};
        trim(title_str);
        parsed += (err ? nullstr : title_str) + "\t"s;

        err = job["skills"].get(skills);
        parsed += (err ? "[]"s : parse_skills(skills)) + "\t"s;

        err = job["salary"].get(salary);
        parsed += (err ? (nullstr + "\t"s + "[]"s) : parse_salary(salary)) +
                  "\t"s;

        err = job["locations"].get(locations);
        parsed += (err ? "[]"s : parse_locations(locations)) + "\t"s;

        err = job["remoteWork"].get(remote);
        parsed += (err ? nullstr : std::to_string(remote));

        return std::move(parsed + "\n"s);
}

std::vector<std::future<std::string>>
moikrug::transform(std::vector<std::future<std::string>> input) {
        std::string data{};

        for (auto &future : input) {
                auto datum = future.get();

                data += datum + "\n"s;
        }

        data = std::string(simdjson::padded_string(data));
        simdjson::dom::document_stream pages = parser.parse_many(data);

        std::vector<std::string> parsed{};

        switch (parse_mode) {
        case mode::meta:
                for (simdjson::dom::element page : pages) {
                        auto meta = page["meta"];

                        auto pages_total = parse_pages_total(meta);

                        parsed.push_back(pages_total);

                        break;
                }

                parse_mode = mode::job;
                break;
        case mode::job:
                for (simdjson::dom::element page : pages) {
                        simdjson::dom::array jobs = page["list"];

                        for (auto job_element : jobs) {
                                simdjson::dom::object job;
                                auto err = job_element.get(job);

                                if (err)
                                        std::cerr << err << std::endl;

                                auto parsed_job = parse_job(job);
                                parsed.push_back(parsed_job);
                        }
                }

                parse_mode = mode::meta;
                break;
        }

        std::vector<std::future<std::string>> output{};

        for (auto data : parsed) {
                output.push_back(
                        std::async(std::launch::async, [=]() { return data; }));
        }

        return std::move(output);
}

moikrug::moikrug() : parse_mode{moikrug::mode::meta} {}

} // namespace transformer
} // namespace ETL
