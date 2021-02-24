#pragma once

#include "transformer.h"

#include <algorithm>

#include "../lib/simdjson/main.h"

namespace ETL {
namespace transformer {

using namespace simdjson;
using namespace std::string_literals;
using namespace std::literals::string_view_literals;

class hh : public base {
    public:
        hh();

        std::vector<std::future<std::string>>
        transform(std::vector<std::future<std::string>> input);

    private:
        enum class mode { pages, links, job };

        mode parse_mode;

        dom::parser parser;
};

hh::hh() : parse_mode{hh::mode::pages} {}

std::vector<std::future<std::string>>
hh::transform(std::vector<std::future<std::string>> input) {
        std::vector<std::future<std::string>> output{};

        std::vector<std::string> parsed{};

        std::string data{};

        for (auto &future : input) {
                auto datum = future.get();

                data += datum + "\n"s;
        }

        data = std::string{padded_string(data)};

        dom::document_stream docs = parser.parse_many(data);

        switch (parse_mode) {
        case mode::pages: {
                for (dom::element chunk : docs) {
                        auto n = std::to_string(
                                static_cast<size_t>(chunk["pages"]));

                        parsed.push_back(n);
                }

                parse_mode = mode::links;

                break;
        }
        case mode::links: {
                for (dom::element page : docs) {
                        dom::array jobs = page["items"];

                        for (auto job_element : jobs) {
                                dom::object job;

                                auto err = job_element.get(job);
                                if (err) {
                                        std::cerr << err << std::endl;

                                        continue;
                                }

                                std::string_view url{};

                                err = job["url"].get(url);
                                if (err) {
                                        std::cerr << err << std::endl;
                                        continue;
                                }

                                parsed.push_back(std::string{url});
                        }
                }

                parse_mode = mode::job;

                break;
        }
        // TODO: refactor
        //       decompose into separate functions
        case mode::job: {
                for (dom::element job : docs) {
                        std::string row{};

                        std::string_view id;

                        std::string_view date;

                        std::string_view name;

                        dom::array dom_skills;

                        dom::object salary;

                        dom::object area;

                        dom::object schedule;

                        std::vector<std::string_view> skills{};

                        std::string skills_joined{};

                        auto err = job["id"].get(id);
                        if (err || !id.length())
                                continue;

                        err = job["published_at"].get(date);
                        if (err || !date.length())
                                continue;

                        date = date.substr(0, date.find("T"));

                        row += std::string{id} + "\t" + std::string{date} +
                               "\t";

                        err = job["name"].get(name);

                        row += (name.length() ? std::string{name} : "\\N"s) +
                               "\t";

                        err = job["key_skills"].get(dom_skills);
                        for (auto skill_element : dom_skills) {
                                std::string_view skill;

                                err = skill_element["name"].get(skill);
                                if (!err && skill.length())
                                        skills.push_back(skill);
                        }

                        skills_joined += "["s;

                        for (auto it = skills.begin(); it != skills.end();
                             it++) {
                                skills_joined += "'"s + std::string{*it} + "'"s;

                                if (it + 1 < skills.end())
                                        skills_joined += ","s;
                        }

                        skills_joined += "]"s;

                        row += (skills.size() ? skills_joined : "[]") + "\t";

                        std::string currency_salary{"\\N\t[]"};

                        err = job["salary"].get(salary);
                        if (!err) {
                                size_t from;
                                err = salary["from"].get(from);
                                if (err)
                                        from = 0;

                                size_t to;
                                err = salary["to"].get(to);
                                if (err)
                                        to = 0;

                                std::string_view currency;
                                err = salary["currency"].get(currency);

                                std::string curr_str{};
                                if (!err)
                                        curr_str += currency;

                                std::transform(curr_str.begin(), curr_str.end(),
                                               curr_str.begin(),
                                               [](unsigned char c) {
                                                       return std::tolower(c);
                                               });

                                if (!err && (from || to)) {
                                        if (from && !to)
                                                to = from;
                                        if (!from && to)
                                                from = to;

                                        currency_salary =
                                                curr_str + "\t" + "["s +
                                                std::to_string(from) + ","s +
                                                std::to_string(to) + "]"s;
                                }
                        }

                        row += currency_salary + "\t";

                        std::string location{"[]"};

                        err = job["area"].get(area);
                        if (!err) {
                                std::string_view city;

                                err = area["name"].get(city);
                                if (!err)
                                        location = "["s + "'" +
                                                   std::string{city} + "'" +
                                                   "]"s;
                        }

                        row += location + "\t";

                        std::string remote{"0"};

                        err = job["schedule"].get(schedule);
                        if (!err) {
                                std::string_view id;

                                err = schedule["id"].get(id);
                                if (!err)
                                        remote = std::to_string(
                                                !id.compare("remote"));
                        }

                        row += remote + "\n";

                        parsed.push_back(row);
                }

                break;
        }
        }

        for (auto data : parsed)
                output.push_back(
                        std::async(std::launch::async, [=]() { return data; }));

        return std::move(output);
}

} // namespace transformer
} // namespace ETL
