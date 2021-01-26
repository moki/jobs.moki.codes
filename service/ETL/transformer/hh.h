#pragma once

#include "transformer.h"
#include <algorithm>
#include <cstring>
#include <deque>
#include <numeric>
#include <string>

#include <iostream>

#include "gumbo.h"

#include <iterator>
#include <sstream>

#include <stdexcept>

template <typename Out>
void split(const std::string &s, char delim, Out result) {
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, delim)) {
                if (!item.empty())
                        *result++ = item;
        }
}

std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return std::move(elems);
}

template <typename T> constexpr auto type_name() noexcept {
        std::string_view name = "Error: unsupported compiler", prefix, suffix;
#ifdef __clang__
        name = __PRETTY_FUNCTION__;
        prefix = "auto type_name() [T = ";
        suffix = "]";
#elif defined(__GNUC__)
        name = __PRETTY_FUNCTION__;
        prefix = "constexpr auto type_name() [with T = ";
        suffix = "]";
#elif defined(_MSC_VER)
        name = __FUNCSIG__;
        prefix = "auto __cdecl type_name<";
        suffix = ">(void) noexcept";
#endif
        name.remove_prefix(prefix.size());
        name.remove_suffix(suffix.size());
        return name;
}

using namespace std::string_literals;

namespace ETL {
namespace transformer {

class hh : public base {
    public:
        enum class strategy { css_class };
        enum class mode { meta, links, job };

        hh();
        std::future<std::string> transform(std::string_view &&message);
        void dom_walker(auto node, auto fn);
        void add_strategy_selector(auto strategy_selector);
        void set_parse_mode(mode mode);

    private:
        std::vector<std::pair<strategy, std::string>> strategies_selectors;
        std::unordered_map<std::string, std::vector<GumboInternalNode *>>
                nodes_by_class;
        mode parse_mode;

        std::string parse_page_number(auto node);
        size_t parse_pages_total();

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

        static inline auto select_with_class = [](auto self, auto node,
                                                  auto target_class_name) {
                if (node->type != GUMBO_NODE_ELEMENT)
                        return;

                auto attrs = node->v.element.attributes;
                if (!attrs.length)
                        return;

                auto class_attr = gumbo_get_attribute(&attrs, "class");
                if (!class_attr)
                        return;

                auto class_names = split(std::string{class_attr->value}, ' ');
                auto selector = std::make_pair(strategy::css_class, ""s);

                for (auto &class_name : class_names) {
                        auto matches = class_name == target_class_name;
                        if (matches) {
                                auto nodes_it = self->nodes_by_class.find(
                                        target_class_name);
                                if (nodes_it != self->nodes_by_class.end())
                                        nodes_it->second.push_back(node);
                        }
                }
        };

        static inline auto select = [](auto self, auto node) {
                for (auto strategy_selector : self->strategies_selectors) {
                        switch (strategy_selector.first) {
                        case strategy::css_class:
                                select_with_class(self, node,
                                                  strategy_selector.second);
                                break;
                        }
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

        for (auto matched_nodes_by_class : nodes_by_class) {
                auto class_name = matched_nodes_by_class.first;
                auto matched_nodes = matched_nodes_by_class.second;

                for (auto node : matched_nodes) {
                        size_t page_number;
                        try {
                                page_number =
                                        std::stoll(parse_page_number(node));
                        } catch (std::exception &e) {
                                page_number = 0;
                        }

                        if (page_number > pages_total)
                                pages_total = page_number;
                }
        }

        return std::move(pages_total);
}

std::future<std::string> hh::transform(std::string_view &&html) {
        return std::async(std::launch::async, [&]() {
                GumboOptions options = kGumboDefaultOptions;
                GumboOutput *output = gumbo_parse_with_options(
                        &options, html.data(), html.length());

                /*
                for (auto matched_nodes_by_class : nodes_by_class) {
                        auto class_name = matched_nodes_by_class.first;
                        auto matched_nodes = matched_nodes_by_class.second;

                        std::cout
                                << type_name<decltype(matched_nodes_by_class)>()
                                << std::endl;

                        std::cout << "found: " << matched_nodes.size()
                                  << std::endl;
                        std::cout << "nodes with class: " << class_name
                                  << std::endl;

                        for (auto node : matched_nodes) {
                                std::cout << "node: "
                                          << gumbo_normalized_tagname(
                                                     node->v.element.tag)
                                          << std::endl;
                                std::cout << parse_page_number(node)
                                          << std::endl;
                        }
                }
                */

                std::string parsed{};

                switch (parse_mode) {
                case mode::meta: {
                        add_strategy_selector(std::make_pair(
                                strategy::css_class, "HH-Pager-Control"));

                        dom_walker(output->root, select);

                        parsed = std::to_string(parse_pages_total());

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

void hh::add_strategy_selector(auto strategy_selector) {
        switch (strategy_selector.first) {
        case hh::strategy::css_class:
                strategies_selectors.push_back(strategy_selector);
                nodes_by_class.insert({strategy_selector.second,
                                       std::vector<GumboInternalNode *>{}});
                break;
        default:
                std::cerr << "strategy not implemented" << std::endl;
                exit(1);
        }
}

hh::hh() : parse_mode{hh::mode::meta} {}

} // namespace transformer
} // namespace ETL
