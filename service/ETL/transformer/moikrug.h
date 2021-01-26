#pragma once

#include "transformer.h"
#include <numeric>
#include <string>

#include "../lib/simdjson/simdjson.cpp"
// #include "../lib/simdjson/simdjson.h"

using namespace std::string_literals;

namespace ETL {
namespace transformer {

class moikrug : public base {
    public:
        moikrug();
        std::future<std::string> transform(std::string_view &&message);

        bool metaparser;

    private:
        std::string parse_meta(auto &);
        std::string parse_job(auto &);
        std::string parse_skills(auto &);
        std::string parse_salary(auto &);
        std::string_view parse_date(auto &);
        std::string parse_locations(auto &);
        simdjson::dom::parser parser;
        std::string nullstr;
};

std::string moikrug::parse_meta(auto &meta) {
        metaparser = false;

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
                skills_vec.push_back(std::string{skill_str});
        }

        if (!skills_vec.size())
                return nullstr;

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
                return nullstr + "\t"s + nullstr /*"[0,0]"s*/;

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
                // range += "0,0]"s;
                range = nullstr;
                break;
        }
        }

        return std::move(std::string{currency} + "\t"s + range);
}

std::string moikrug::parse_locations(auto &locations) {
        std::vector<std::string_view> titles;

        for (simdjson::dom::element location_element : locations) {
                simdjson::dom::object location;
                std::string_view title;

                auto err = location_element.get(location);
                if (err)
                        break;

                err = location["title"].get(title);
                titles.push_back(title);
        }

        if (!titles.size())
                return nullstr;

        return std::accumulate(titles.begin() + 1, titles.end(),
                               std::move("["s + std::string{*(titles.begin())}),
                               [](std::string_view a, std::string_view b) {
                                       return std::move(std::string{a} + "," +
                                                        std::string{b});
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
        parsed += (err ? nullstr : std::string{title}) + "\t"s;

        err = job["skills"].get(skills);
        parsed += (err ? nullstr : parse_skills(skills)) + "\t"s;

        err = job["salary"].get(salary);
        parsed += (err ? (nullstr + "\t"s + "[0,0]"s) : parse_salary(salary)) +
                  "\t"s;

        err = job["locations"].get(locations);
        parsed += (err ? nullstr : parse_locations(locations)) + "\t"s;

        err = job["remoteWork"].get(remote);
        parsed += (err ? nullstr : std::to_string(remote));

        return std::move(parsed + "\n"s);
}

std::future<std::string> moikrug::transform(std::string_view &&message) {
        return std::async(std::launch::async, [&]() {
                simdjson::dom::array pages;
                std::string job_data{};
                size_t i = 0;

                auto padded = std::string(simdjson::padded_string(message));
                auto err = parser.parse(padded).get(pages);

                if (err) {
                        std::cerr << err << std::endl;
                        return ""s;
                }

                for (auto page : pages) {
                        auto meta = page["meta"];

                        if (metaparser)
                                return parse_meta(meta);

                        simdjson::dom::array jobs = page["list"];

                        for (auto job_element : jobs) {
                                simdjson::dom::object job;
                                err = job_element.get(job);

                                if (err) {
                                        std::cerr << err << std::endl;
                                        return std::move(std::string(message));
                                }

                                job_data += parse_job(job);
                        }
                }

                return std::move(job_data);
        });
}

moikrug::moikrug() : metaparser{true}, nullstr{"\\N"} {}

} // namespace transformer
} // namespace ETL
