#pragma once

#include "transformer.h"

#include "../debug/type_name.h"
#include "../utility/split.h"

#include "gumbo.h"

#include <deque>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace ETL {
namespace transformer {

struct hash_pair {
        template <typename T1, typename T2>
        size_t operator()(const std::pair<T1, T2> &pair) const {
                return std::hash<T1>()(pair.first + pair.second);
        }
};

class html : public base {
    public:
        html();

    protected:
        std::vector<std::pair<std::string, std::string>> selectors;
        std::unordered_map<std::pair<std::string, std::string>,
                           std::vector<GumboInternalNode *>, hash_pair>
                nodes_by_selector;

        void add_selector(auto selector);

        void dom_constructor(std::string from);
        void dom_parse();
        void dom_destructor();

        auto node_find_direct_child(auto node, auto selector);

        void set_debug_flag(bool flag);

    private:
        GumboOutput *dom;
        GumboOptions default_dom_options;

        bool debug;

        void dom_walker(auto node, auto fn);

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

                auto attr_key = selector.first.c_str();

                auto attr = gumbo_get_attribute(&attrs, attr_key);
                if (!attr)
                        return;

                auto attr_str = std::string{attr->value};
                auto attr_tokens = split(attr_str, ' ');

                for (auto &attr_token : attr_tokens) {
                        if (!attr_token.compare(selector.second)) {
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

                if (self->debug)
                        log_node(self, node);
        };
};

void html::add_selector(auto selector) {
        selectors.push_back(selector);
        nodes_by_selector.insert(
                {selector, std::vector<GumboInternalNode *>{}});
}

void html::dom_walker(auto node, auto fn) {
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

auto html::node_find_direct_child(auto node, auto selector) {
        GumboNode *cursor = nullptr;

        if (node->type != GUMBO_NODE_ELEMENT)
                return cursor;

        auto children = node->v.element.children.data;
        auto children_length = node->v.element.children.length;

        for (size_t i = 0; i < children_length; i++) {
                cursor = static_cast<GumboNode *>(children[i]);

                if (cursor->type != GUMBO_NODE_ELEMENT)
                        continue;

                auto attributes = cursor->v.element.attributes;
                if (!attributes.length)
                        continue;

                auto dom_attr = gumbo_get_attribute(&attributes,
                                                    selector->first.c_str());
                if (!dom_attr)
                        continue;

                auto value = std::string{dom_attr->value};
                if (!value.compare(selector->second))
                        break;
        }

        return cursor;
}

void html::dom_parse() { dom_walker(dom->root, select); }

void html::dom_constructor(std::string from) {
        default_dom_options = kGumboDefaultOptions;

        dom = gumbo_parse_with_options(&default_dom_options, from.data(),
                                       from.length());
}

void html::dom_destructor() { gumbo_destroy_output(&default_dom_options, dom); }

void html::set_debug_flag(bool flag) { debug = flag; }

html::html() : base{}, debug{false} {}

} // namespace transformer
} // namespace ETL
