#pragma once

#include <algorithm>
#include <cwctype>
#include <deque>
#include <iostream>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>

#include <unordered_map>
#include <utility>

#include "gumbo.h"

#include "transformer.h"

#include "../../common/debug/type_name.h"
#include "../../common/utility/split.h"
#include "../../common/utility/trim.h"

using namespace std::string_literals;

namespace ETL {
namespace transformer {

struct hash_pair {
        template <typename T1, typename T2>
        size_t operator()(const std::pair<T1, T2> &pair) const {
                return std::hash<T1>()(pair.first) ^
                       std::hash<T2>()(pair.second);
        }
};

class hh : public base {
    public:
        enum class mode { meta, links, job };

        hh();

        std::future<std::string> transform(std::string_view &&message);

        void set_parse_mode(mode mode);

    private:
        /* meta classes */
        static inline std::string paginator_class = "HH-Pager-Control"s;
        /* links classes */
        static inline std::string job_link_class = "HH-LinkModifier"s;
        /* job data classes */
        static inline std::string job_title_class = "vacancy-title"s;
        static inline std::string job_salary_class = "vacancy-salary"s;
        static inline std::string job_skills_class = "bloko-tag_inline"s;
        static inline std::string job_id_class = "vacancy-action_stretched"s;
        static inline std::string job_remote_attr =
                "vacancy-view-employment-mode"s;
        static inline std::string job_date_class = "vacancy-creation-time"s;
        static inline std::string job_location_attr = "vacancy-view-location"s;

        static inline std::unordered_map<std::string, std::string> months{
                {"января", "01"},  {"февраля", "02"}, {"марта", "03"},
                {"апреля", "04"},  {"мая", "05"},     {"июня", "06"},
                {"июля", "07"},    {"августа", "08"}, {"сентября", "09"},
                {"октября", "10"}, {"ноября", "11"},  {"декабря", "12"}};

        std::vector<std::pair<std::string, std::string>> selectors;
        std::unordered_map<std::pair<std::string, std::string>,
                           std::vector<GumboInternalNode *>, hash_pair>
                nodes_by_selector;
        mode parse_mode;

        void dom_walker(auto node, auto fn);
        void add_selector(auto selector);

        std::string parse_page_number(auto node);
        size_t parse_pages_total();

        std::string parse_job_url(auto node);
        std::vector<std::string> parse_job_urls();
        std::string parse_job_title();
        std::string parse_job_salary();
        std::string parse_job_id();
        std::string parse_job_date();
        std::string parse_job_skills();
        std::string parse_job_location();
        std::string parse_job_remote();

        static inline auto log_node = [](auto self, auto node) {
                std::cout << type_name<decltype(node)>() << std::endl;

                switch (node->type) {
                case GUMBO_NODE_ELEMENT:
                        std::cout << gumbo_normalized_tagname(
                                node->v.element.tag);
                        break;
                case GUMBO_NODE_TEXT:
                        std::cout << std::string(node->v.text.text);
                        break;
                default:
                        std::cout << "unknown dom element";
                }

                std::cout << std::endl;
        };

        static inline auto select_with_selector = [](auto self, auto node,
                                                     auto selector) {
                if (node->type != GUMBO_NODE_ELEMENT)
                        return;

                auto attrs = node->v.element.attributes;
                if (!attrs.length)
                        return;

                auto attr = gumbo_get_attribute(&attrs, selector.first.c_str());
                if (!attr)
                        return;

                auto attr_tokens = split(std::string{attr->value}, ' ');

                for (auto &attr_token : attr_tokens) {
                        if (attr_token == selector.second) {
                                auto nodes_it =
                                        self->nodes_by_selector.find(selector);
                                if (nodes_it != self->nodes_by_selector.end())
                                        nodes_it->second.push_back(node);
                        }
                }
        };

        static inline auto select = [](auto self, auto node) {
                for (auto selector : self->selectors) {
                        select_with_selector(self, node, selector);
                }
                // log_node(self, node);
        };
};

void hh::dom_walker(auto node, auto fn) {
        std::deque<GumboNode *> stack{};
        stack.push_back(node);

        for (; stack.size();) {
                auto node = stack.back();
                stack.pop_back();

                if (node->type != GUMBO_NODE_ELEMENT)
                        continue;

                auto children_size = node->v.element.children.length;
                auto children = node->v.element.children.data;

                for (; children_size--;)
                        stack.push_back((GumboNode *)children[children_size]);

                fn(this, node);
        }
}

std::string hh::parse_page_number(auto node) {
        std::string s{};

        auto children = node->v.element.children.data;
        auto children_size = node->v.element.children.length;

        if (children_size != 1)
                return std::move(s);

        auto first_child = static_cast<GumboNode *>(children[0]);
        if (first_child->type != GUMBO_NODE_TEXT)
                return std::move(s);

        s += first_child->v.text.text;

        return std::move(s);
}

size_t hh::parse_pages_total() {
        size_t pages_total = 0;

        auto selector = std::make_pair("class"s, paginator_class);

        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(pages_total);
        if (!it->second.size())
                return std::move(pages_total);

        auto nodes = it->second;

        for (auto node : nodes) {
                size_t page_number;

                try {
                        page_number = std::stoll(parse_page_number(node));
                } catch (std::exception &e) {
                        page_number = 0;
                }

                if (page_number > pages_total)
                        pages_total = page_number;
        }

        return std::move(pages_total);
}

std::string hh::parse_job_url(auto node) {
        std::string s{};

        if (node->type != GUMBO_NODE_ELEMENT)
                return std::move(s);

        auto attrs = node->v.element.attributes;
        if (!attrs.length)
                return std::move(s);

        auto href_attr = gumbo_get_attribute(&attrs, "href");
        if (!href_attr)
                return std::move(s);

        s = std::string{href_attr->value};

        return std::move(s);
}

std::vector<std::string> hh::parse_job_urls() {
        std::vector<std::string> job_urls{};

        auto selector = std::make_pair("class"s, job_link_class);

        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(job_urls);
        if (!it->second.size())
                return std::move(job_urls);

        auto nodes = it->second;

        for (auto node : nodes)
                job_urls.push_back(parse_job_url(node));

        return std::move(job_urls);
}

std::string hh::parse_job_title() {
        std::string title{"\\N"};

        auto selector = std::make_pair("class"s, job_title_class);

        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(title);
        if (!it->second.size())
                return std::move(title);

        auto div_node = it->second[0];

        if (div_node->type != GUMBO_NODE_ELEMENT)
                return std::move(title);

        if (!div_node->v.element.children.length)
                return std::move(title);

        auto title_node =
                static_cast<GumboNode *>(div_node->v.element.children.data[0]);
        if (title_node->type != GUMBO_NODE_ELEMENT)
                return std::move(title);

        if (!title_node->v.element.children.length)
                return std::move(title);

        auto title_text_node = static_cast<GumboNode *>(
                title_node->v.element.children.data[0]);

        if (title_text_node->type != GUMBO_NODE_TEXT)
                return std::move(title);

        title = title_text_node->v.text.text;

        return std::move(title);
}

std::string hh::parse_job_salary() {
        std::string salary{};

        auto selector = std::make_pair("class"s, job_salary_class);

        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(salary);
        if (!it->second.size())
                return std::move(salary);

        auto p_node = it->second[0];

        if (p_node->type != GUMBO_NODE_ELEMENT)
                return std::move(salary);

        if (!p_node->v.element.children.length)
                return std::move(salary);

        auto salary_node =
                static_cast<GumboNode *>(p_node->v.element.children.data[0]);
        if (salary_node->type != GUMBO_NODE_ELEMENT)
                return std::move(salary);

        auto salary_text_nodes = salary_node->v.element.children.data;
        auto salary_text_nodes_size = salary_node->v.element.children.length;

        if (!salary_text_nodes_size)
                return std::move(salary);

        size_t i;
        bool is_currency_set = false;
        std::string currency{"\\N"};
        std::vector<size_t> salary_range;

        for (i = 0; i < salary_text_nodes_size; ++i) {
                auto salary_text_node =
                        static_cast<GumboNode *>(salary_text_nodes[i]);

                if (salary_text_node->type != GUMBO_NODE_TEXT)
                        continue;

                auto salary_token = std::string{salary_text_node->v.text.text};

                salary_token = [token = std::move(salary_token)]() {
                        std::string escaped{};

                        for (size_t c : token) {
                                switch (c) {
                                case 18446744073709551554u:
                                case 18446744073709551520u:
                                case ' ':
                                case '\n':
                                case '\t':
                                        break;
                                default:
                                        escaped += std::tolower(c);
                                }
                        }

                        return std::move(escaped);
                }();


                try {
                        auto range = std::stoll(salary_token);

                        salary_range.push_back(range);

                        continue;
                } catch (std::exception &e) {
                }

                if (!salary_token.compare("руб.")) {
                        currency = "rur"s;
                        is_currency_set = true;

                        continue;
                }

                if (!salary_token.compare("usd")) {
                        currency = "usd"s;
                        is_currency_set = true;

                        continue;
                }
        }

        salary += currency + "\t"s;

        switch (salary_range.size()) {
        case 0:
                salary += "\\N"s;
                break;
        case 1:
                salary += "["s + std::to_string(salary_range[0]) + ","s +
                          std::to_string(salary_range[0]) + "]"s;
                break;
        case 2:
                salary += "["s + std::to_string(salary_range[0]) + ","s +
                          std::to_string(salary_range[1]) + "]"s;
                break;
        }

        return std::move(salary);
}

std::string hh::parse_job_id() {
        std::string id{};

        auto selector = std::make_pair("class"s, job_id_class);

        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(id);
        if (!it->second.size())
                return std::move(id);

        auto div_node = it->second[0];

        if (div_node->type != GUMBO_NODE_ELEMENT)
                return std::move(id);

        if (!div_node->v.element.children.length)
                return std::move(id);

        auto id_node =
                static_cast<GumboNode *>(div_node->v.element.children.data[0]);
        if (id_node->type != GUMBO_NODE_ELEMENT)
                return std::move(id);

        auto id_attrs = id_node->v.element.attributes;
        if (!id_attrs.length)
                return std::move(id);

        auto href = gumbo_get_attribute(&id_attrs, "href");
        if (!href)
                return std::move(id);

        auto href_str = std::string{href->value};

        auto href_id_start = href_str.find("Id="s);
        if (href_id_start == std::string::npos)
                return std::move(id);

        auto href_id_end = href_str.find("&"s, href_id_start + 3);
        if (href_id_end == std::string::npos)
                return std::move(id);

        id = href_str.substr(href_id_start + 3,
                             href_id_end - (href_id_start + 3));

        return std::move(id);
}

std::string hh::parse_job_date() {
        std::string date{};

        auto selector = std::make_pair("class"s, job_date_class);

        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(date);
        if (!it->second.size())
                return std::move(date);

        auto p_node = it->second[0];

        if (p_node->type != GUMBO_NODE_ELEMENT)
                return std::move(date);

        auto children = p_node->v.element.children.data;
        auto children_size = p_node->v.element.children.length;

        if (children_size < 2)
                return std::move(date);

        auto date_str =
                std::string{static_cast<GumboNode *>(children[2])->v.text.text};

        date = [date = std::move(date_str)]() {
                std::string escaped{};
                std::vector<std::string> tokens{};

                auto token_it = date.begin();
                for (auto it = date.begin(); it != date.end() - 2; it += 2) {
                        size_t first = *(it);
                        size_t second = *(it + 1);

                        if (first == 18446744073709551554u &&
                            second == 18446744073709551520u) {
                                tokens.push_back(std::string(
                                        token_it, token_it + (it - token_it)));
                                token_it = it + 2;
                        }
                }

                tokens.push_back(std::string(token_it, date.end()));

                if (tokens.size() != 3)
                        return std::move(escaped);

                auto year = tokens[2];
                auto day = tokens[0];
                auto month_it = months.find(tokens[1]);
                if (month_it == months.end())
                        return std::move(escaped);

                escaped += tokens[2] + '-' + month_it->second + '-' + tokens[0];

                return std::move(escaped);
        }();

        return std::move(date);
}

std::string hh::parse_job_skills() {
        std::string null_str{"\\N"};

        auto selector = std::make_pair("class"s, job_skills_class);

        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(null_str);
        if (!it->second.size())
                return std::move(null_str);

        std::vector<std::string> skills;

        for (auto div_node : it->second) {
                if (div_node->type != GUMBO_NODE_ELEMENT)
                        continue;

                if (!div_node->v.element.children.length)
                        continue;

                auto span_node = static_cast<GumboNode *>(
                        div_node->v.element.children.data[0]);

                if (!span_node->v.element.children.length)
                        continue;

                auto text_node = static_cast<GumboNode *>(
                        span_node->v.element.children.data[0]);

                if (text_node->type != GUMBO_NODE_TEXT)
                        continue;

                std::string skill{};
                std::string skill_raw{text_node->v.text.text};

                for (wchar_t c : skill_raw) {
                        if (!std::iswpunct(c) && !std::iswcntrl(c))
                                skill += c;
                }

                skills.push_back(skill);
        }

        if (!skills.size())
                return std::move(null_str);

        return std::accumulate(skills.begin() + 1, skills.end(),
                               std::move("["s + skills[0]),
                               [&](std::string a, std::string b) {
                                       return std::move(a + ","s + b);
                               }) +
               "]"s;
}

std::string hh::parse_job_location() {
        std::string location{"\\N"};

        auto selector = std::make_pair("data-qa"s, job_location_attr);

        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(location);
        if (!it->second.size())
                return std::move(location);

        auto p_node = it->second[0];
        if (p_node->type != GUMBO_NODE_ELEMENT)
                return std::move(location);

        if (!p_node->v.element.children.length)
                return std::move(location);

        auto text_node =
                static_cast<GumboNode *>(p_node->v.element.children.data[0]);
        if (text_node->type != GUMBO_NODE_TEXT)
                return std::move(location);

        location = std::string{"["s + text_node->v.text.text + "]"s};

        return std::move(location);
}

std::string hh::parse_job_remote() {
        auto remote = "\\N"s;

        auto it = nodes_by_selector.find(
                std::make_pair("data-qa"s, job_remote_attr));
        if (it == nodes_by_selector.end())
                return std::move(remote);
        if (!it->second.size())
                return std::move(remote);

        auto p_node = it->second[0];
        if (p_node->type != GUMBO_NODE_ELEMENT)
                return std::move(remote);

        auto children = p_node->v.element.children.data;
        auto children_length = p_node->v.element.children.length;

        if (!children_length)
                return std::move(remote);

        for (size_t i = 0; i < children_length; ++i) {
                auto child = static_cast<GumboNode *>(children[i]);
                if (child->type != GUMBO_NODE_ELEMENT)
                        continue;

                if (!child->v.element.children.length)
                        continue;

                auto text_node = static_cast<GumboNode *>(
                        child->v.element.children.data[0]);
                if (text_node->type != GUMBO_NODE_TEXT)
                        continue;

                auto remote_text = std::string{text_node->v.text.text};

                if (!remote_text.length())
                        continue;

                remote = std::to_string(
                        remote_text.compare("удаленная работа"s) == 0);
        }

        return std::move(remote);
}

std::future<std::string> hh::transform(std::string_view &&html) {
        return std::async(std::launch::async, [&]() {
                GumboOptions options = kGumboDefaultOptions;
                GumboOutput *output = gumbo_parse_with_options(
                        &options, html.data(), html.length());

                std::string parsed{};

                switch (parse_mode) {
                case mode::meta: {
                        add_selector(std::make_pair("class"s, paginator_class));

                        dom_walker(output->root, select);

                        parsed = std::to_string(parse_pages_total());

                        parse_mode = mode::links;
                        selectors.clear();
                        nodes_by_selector.clear();

                        break;
                }
                case mode::links: {
                        add_selector(std::make_pair("class"s, job_link_class));

                        dom_walker(output->root, select);

                        auto job_urls = parse_job_urls();

                        parse_mode = mode::job;
                        selectors.clear();
                        nodes_by_selector.clear();

                        if (!job_urls.size())
                                return std::move(parsed);

                        parsed += *(job_urls.begin());
                        for (auto it = job_urls.begin() + 1;
                             it != job_urls.end(); ++it) {
                                parsed += ("\n"s + *it);
                        }

                        break;
                }
                case mode::job: {
                        add_selector(std::make_pair("class"s, job_title_class));

                        add_selector(
                                std::make_pair("class"s, job_salary_class));

                        add_selector(std::make_pair("class"s, job_id_class));

                        add_selector(std::make_pair("class"s, job_date_class));

                        add_selector(
                                std::make_pair("class"s, job_skills_class));

                        add_selector(
                                std::make_pair("class"s, job_location_attr));

                        add_selector(
                                std::make_pair("data-qa"s, job_location_attr));

                        add_selector(
                                std::make_pair("data-qa"s, job_remote_attr));

                        dom_walker(output->root, select);

                        auto job_title = parse_job_title();
                        auto job_salary = parse_job_salary();
                        auto job_id = parse_job_id();
                        auto job_date = parse_job_date();
                        auto job_skills = parse_job_skills();
                        auto job_location = parse_job_location();
                        auto job_remote = parse_job_remote();

                        parse_mode = mode::meta;
                        selectors.clear();
                        nodes_by_selector.clear();

                        if (!job_id.length() || !job_date.length())
                                break;

                        parsed += job_id + "\t"s;

                        parsed += job_date + "\t"s;

                        parsed += job_title + "\t"s;

                        parsed += job_skills + "\t"s;

                        parsed += job_salary + "\t"s;

                        parsed += job_location + "\t"s;

                        parsed += job_remote + "\n"s;

                        break;
                }
                default: {
                        std::cerr << "parse mode not implemented" << std::endl;
                        exit(1);
                }
                }

                gumbo_destroy_output(&options, output);

                return parsed;
        });
}

void hh::set_parse_mode(mode mode) { parse_mode = mode; }

void hh::add_selector(auto selector) {
        selectors.push_back(selector);
        nodes_by_selector.insert(
                {selector, std::vector<GumboInternalNode *>{}});
}

hh::hh() : parse_mode{hh::mode::meta} {}

} // namespace transformer
} // namespace ETL
