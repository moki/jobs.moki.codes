#include "html.h"

#include <set>

#include "../utility/split.h"
#include "../utility/trim.h"

namespace ETL {
namespace transformer {

using namespace std::string_literals;

class profunctor : public html {
    public:
        enum class mode { meta, job };

        using html::html;

        profunctor();

        std::string parse(std::string);

        std::vector<std::future<std::string>>
        transform(std::vector<std::future<std::string>> input) override;

    private:
        static inline std::string more_messages_class = "tme_messages_more"s;
        static inline std::string more_messages_attribute = "data-before"s;

        static inline std::string message_date_class =
                "tgme_widget_message_date"s;

        static inline std::string message_bubble_class =
                "tgme_widget_message_bubble"s;

        static inline std::string message_text_classes =
                "tgme_widget_message_text js-message_text"s;

        static inline std::string message_footer_classes =
                "tgme_widget_message_footer compact js-message_footer"s;

        static inline std::string message_footer_info_classes =
                "tgme_widget_message_info short js-message_info"s;

        static inline std::string message_footer_info_meta_classes =
                "tgme_widget_message_meta"s;

        static inline std::string message_footer_info_meta_link_classes =
                "tgme_widget_message_date"s;

        static inline std::string message_footer_info_meta_link_time_classes =
                "time"s;

        mode parse_mode;

        // meta
        std::string parse_it();
        std::string parse_step();

        // job
        std::string parse_jobs();
        std::string parse_job(auto node);
        std::string parse_job_meta(auto node);
        std::string parse_job_body(auto node);

        std::set<std::string> ids;
};

profunctor::profunctor() : parse_mode{profunctor::mode::meta} {};

std::string profunctor::parse_it() {
        std::string meta{};

        auto selector = std::make_pair("class"s, more_messages_class);
        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(meta);
        if (!it->second.size())
                return std::move(meta);

        auto nodes = it->second;

        for (auto node : nodes) {
                if (node->type != GUMBO_NODE_ELEMENT)
                        continue;

                auto attributes = node->v.element.attributes;
                if (!attributes.length)
                        continue;

                auto data_attr = gumbo_get_attribute(
                        &attributes, more_messages_attribute.c_str());
                if (!data_attr)
                        continue;

                meta = std::string{data_attr->value};

                break;
        }

        return std::move(meta);
}

std::string profunctor::parse_step() {
        std::string step{};

        auto selector = std::make_pair("class"s, message_date_class);
        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(step);
        if (!it->second.size())
                return std::move(step);

        step = std::to_string(it->second.size());

        return std::move(step);
}

std::string profunctor::parse_job_meta(auto node) {
        std::string meta{};

        if (!node)
                return std::move(meta);

        if (node->type != GUMBO_NODE_ELEMENT)
                return std::move(meta);

        auto info_selector =
                std::make_pair("class"s, message_footer_info_classes);

        auto info = html::node_find_direct_child(node, &info_selector);
        if (!info)
                return std::move(meta);

        auto meta_selector =
                std::make_pair("class"s, message_footer_info_meta_classes);

        auto meta_node = html::node_find_direct_child(info, &meta_selector);
        if (!meta_node)
                return std::move(meta);

        auto link_selector =
                std::make_pair("class"s, message_footer_info_meta_link_classes);

        auto link = html::node_find_direct_child(meta_node, &link_selector);
        if (!link)
                return std::move(meta);

        auto link_attributes = link->v.element.attributes;
        if (!link_attributes.length)
                return std::move(meta);

        auto href_attribute = gumbo_get_attribute(&link_attributes, "href");
        if (!href_attribute)
                return std::move(meta);

        auto href_attribute_value = std::string{href_attribute->value};
        if (!href_attribute_value.length())
                return std::move(meta);

        auto i = href_attribute_value.rfind("/") + 1;
        auto id = href_attribute_value.substr(i, href_attribute_value.length() -
                                                         i);

        auto [it, inserted] = ids.insert(id);
        if (!inserted)
                return std::move(meta);

        auto time_selector = std::make_pair(
                "class"s, message_footer_info_meta_link_time_classes);

        auto time = html::node_find_direct_child(link, &time_selector);
        if (!time)
                return std::move(meta);

        auto time_attributes = time->v.element.attributes;
        if (!time_attributes.length)
                return std::move(meta);

        auto date_attribute = gumbo_get_attribute(&time_attributes, "datetime");
        if (!date_attribute)
                return std::move(meta);

        auto date_attribute_value = std::string{date_attribute->value};
        if (!date_attribute_value.length())
                return std::move(meta);

        i = date_attribute_value.find("T");
        auto date = date_attribute_value.substr(0, i);

        meta = id + "\t"s + date;

        return std::move(meta);
}

std::string profunctor::parse_job_body(auto node) {
        std::string body{};

        if (!node)
                return std::move(body);

        if (node->type != GUMBO_NODE_ELEMENT)
                return std::move(body);

        auto children = node->v.element.children.data;
        auto children_length = node->v.element.children.length;

        std::string job_title{};

        auto title_node = static_cast<GumboNode *>(children[0]);
        if (title_node->type == GUMBO_NODE_ELEMENT) {
                if (!title_node->v.element.children.length)
                        return std::move(body);

                auto text_node = static_cast<GumboNode *>(
                        title_node->v.element.children.data[0]);

                auto title = text_node->v.text.text;
                if (!title)
                        return std::move(body);

                job_title = std::string{title};
        }

        if (title_node->type == GUMBO_NODE_TEXT) {
                auto text = title_node->v.text.text;
                if (!text)
                        return std::move(body);

                job_title = std::string{text};
                if (!job_title.length())
                        return std::move(body);

                auto del = "Тип:"s;

                auto i = job_title.find(del);

                job_title = job_title.substr(i + del.length() + 1,
                                             job_title.length() - i -
                                                     del.length() - 1);
        }

        if (!job_title.length())
                return std::move(body);

        std::string job_skills{};
        std::string job_salary{};
        std::string job_remote{"0"};
        std::string job_location{};

        auto del_skills = "Стэк:"s;
        auto del_salary = "Денег:"s;
        auto del_schedule = "График:"s;
        auto del_legacy = "Легаси:"s;
        auto del_contact = "Контакт:"s;
        auto del_location = "Локация:"s;

        bool ctx_salary = false;

        for (size_t i = 0; i < children_length; i++) {
                auto child = static_cast<GumboNode *>(children[i]);

                if (child->type != GUMBO_NODE_TEXT)
                        continue;

                if (!child->v.text.text)
                        continue;

                auto text = std::string{child->v.text.text};
                if (!text.length())
                        continue;

                size_t index;

                index = text.find(del_skills);
                if (index != std::string::npos) {
                        auto skills_str =
                                text.substr(index + del_skills.length() + 1);

                        auto skills = split(skills_str, ',');

                        if (skills.size())
                                job_skills = "["s;

                        for (auto &skill : skills) {
                                trim(skill);

                                job_skills += "'" + skill + "',";
                        }

                        if (skills.size())
                                job_skills[job_skills.length() - 1] = ']';

                        continue;
                }

                index = text.find(del_salary);
                if (index != std::string::npos) {
                        ctx_salary = true;

                        auto currency = text.find("$") != std::string::npos
                                                ? "usd"s
                                                : "rur"s;

                        auto salary_range = text.substr(
                                index + del_salary.length() + 1 + 1);

                        if (!salary_range.length())
                                continue;

                        auto dash_i = salary_range.find("-");
                        if (dash_i == std::string::npos)
                                continue;

                        auto salary_left = salary_range.substr(0, dash_i);

                        auto salary_right = salary_range.substr(dash_i + 1);

                        job_salary = currency + "\t"s + "["s + salary_left +
                                     "," + salary_right + "]"s;

                        continue;
                }

                index = text.find(del_schedule);
                if (index != std::string::npos)
                        continue;
                index = text.find(del_legacy);
                if (index != std::string::npos)
                        continue;
                index = text.find(del_contact);
                if (index != std::string::npos)
                        continue;

                index = text.find(del_location);
                if (index != std::string::npos) {
                        auto locations_str =
                                text.substr(index + del_location.length() + 1);

                        auto locations = split(locations_str, ',');

                        if (locations.size())
                                job_location = "["s;

                        for (auto &location : locations) {
                                trim(location);
                                job_location += "'" + location + "',";
                        }

                        if (locations.size())
                                job_location[job_location.length() - 1] = ']';

                        ctx_salary = false;

                        break;
                }

                if (ctx_salary) {
                        auto remote = text.find("Remote") != std::string::npos;

                        job_remote = std::to_string(remote);
                        if (remote)
                                break;

                        if (!text.length())
                                break;

                        auto locations = split(text, ',');

                        if (locations.size())
                                job_location = "["s;

                        for (auto &location : locations) {
                                trim(location);
                                job_location += "'" + location + "',";
                        }

                        if (locations.size())
                                job_location[job_location.length() - 1] = ']';

                        break;
                }
        }

        if (!job_skills.length())
                return std::move(body);

        body += job_title + "\t"s;
        body += (job_skills.length() ? job_skills : "[]"s) + "\t"s;
        body += (job_salary.length() ? job_salary : "\\N\t[]"s) + "\t"s;
        body += (job_location.length() ? job_location : "[]"s) + "\t"s;
        body += job_remote;

        return std::move(body);
}

std::string profunctor::parse_job(auto node) {
        std::string job{};

        if (node->type != GUMBO_NODE_ELEMENT)
                return std::move(job);

        auto message_footer_selector =
                std::make_pair("class"s, message_footer_classes);

        auto message_footer =
                html::node_find_direct_child(node, &message_footer_selector);

        if (!message_footer)
                return std::move(job);

        auto meta = parse_job_meta(message_footer);
        if (!meta.length())
                return std::move(job);

        auto message_text_selector =
                std::make_pair("class"s, message_text_classes);

        auto message_text =
                html::node_find_direct_child(node, &message_text_selector);

        if (!message_text)
                return std::move(job);

        auto body = parse_job_body(message_text);
        if (!body.length())
                return std::move(job);

        job = meta + '\t' + body + "\n"s;

        return std::move(job);
}

std::string profunctor::parse_jobs() {
        std::string jobs{};

        auto selector = std::make_pair("class"s, message_bubble_class);
        auto it = nodes_by_selector.find(selector);
        if (it == nodes_by_selector.end())
                return std::move(jobs);
        if (!it->second.size())
                return std::move(jobs);

        auto nodes = it->second;

        for (auto node : nodes)
                jobs += parse_job(node);

        return std::move(jobs);
}

std::string profunctor::parse(std::string html) {
        selectors.clear();
        nodes_by_selector.clear();

        std::string parsed{};

        html::dom_constructor(html);

        switch (parse_mode) {
        case mode::meta: {
                add_selector(std::make_pair("class"s, more_messages_class));
                add_selector(std::make_pair("class"s, message_date_class));

                html::dom_parse();

                auto it = parse_it();

                auto step = parse_step();

                parsed += it + " "s;

                parsed += step;

                break;
        }
        case mode::job: {
                add_selector(std::make_pair("class"s, message_bubble_class));

                html::dom_parse();

                auto jobs = parse_jobs();

                parsed += jobs;

                break;
        }
        }

        html::dom_destructor();

        return std::move(parsed);
}

std::vector<std::future<std::string>>
profunctor::transform(std::vector<std::future<std::string>> input) {
        std::vector<std::string> parsed;
        parsed.reserve(input.size());

        std::vector<std::future<std::string>> output;
        output.reserve(input.size());

        for (auto &future : input) {
                auto html = future.get();

                parsed.push_back(parse(html));
        }

        switch (parse_mode) {
        case mode::meta: {
                auto datum = parsed.back();

                output.push_back(std::async(std::launch::async,
                                            [=]() { return datum; }));

                parse_mode = mode::job;
                break;
        }
        case mode::job: {
                for (auto data : parsed) {
                        output.push_back(std::async(std::launch::async,
                                                    [=]() { return data; }));
                }

                break;
        }
        }

        return std::move(output);
}

} // namespace transformer
} // namespace ETL
