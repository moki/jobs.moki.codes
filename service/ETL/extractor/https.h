#pragma once

#include "extractor.h"
#include <future>
#include <iostream>
#include <numeric>
#include <string>

#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPSClientSession.h"

#include "Poco/Poco.h"
#include "Poco/Timespan.h"
#include "Poco/URI.h"

namespace ETL {
namespace extractor {

using namespace std::string_literals;

static inline auto is_valid_response(std::string &res) {
        return res.find("errors") == std::string::npos;
}

class https : public virtual base {
        inline static std::string
        make_req(std::string url, std::string user_agent,
                 std::chrono::milliseconds req_delay) {
                std::string response{"errors"};

                auto status = 0;

                for (; status != 200 || !is_valid_response(response);) {
                        response = "";

                        if (status)
                                std::cout << "retry" << std::endl;

                        std::this_thread::sleep_for(req_delay);

                        auto uri = Poco::URI{url};

                        Poco::Net::HTTPSClientSession session(uri.getHost(),
                                                              uri.getPort());
                        Poco::Net::HTTPRequest req(
                                Poco::Net::HTTPRequest::HTTP_GET,
                                uri.getPathAndQuery(),
                                Poco::Net::HTTPMessage::HTTP_1_1);

                        req.setKeepAlive(true);

                        auto timeout = Poco::Timespan(10, 0);
                        session.setTimeout(timeout);

                        if (user_agent.length())
                                req.set("User-Agent", user_agent);

                        std::ostream &ostr = session.sendRequest(req);
                        Poco::Net::HTTPResponse res;

                        status = res.getStatus();

                        std::cout << status << std::endl;

                        std::istream &rs = session.receiveResponse(res);
                        // Poco::StreamCopier::copyToString(rs, response);
                        response = std::string(
                                std::istreambuf_iterator<char>(rs), {});

                        std::cout << response << std::endl;
                }

                return response;
        };

    public:
        https(std::string_view user_agent, size_t delay);
        https(std::string_view user_agent);
        https(size_t delay);
        https();

        std::vector<std::future<std::string>> extract() override;
        void add_url(std::string url);
        void reset_urls();
        void set_req_delay(size_t ms);

        std::string user_agent;

    private:
        std::chrono::milliseconds req_delay;
        std::vector<std::string> urls;
};

https::https(std::string_view user_agent, size_t delay)
        : user_agent{user_agent}, req_delay{
                                          std::chrono::milliseconds(delay)} {};

https::https(std::string_view user_agent)
        : user_agent{user_agent}, req_delay{std::chrono::milliseconds(100)} {};

https::https(size_t delay)
        : req_delay{std::chrono::milliseconds(delay)}, user_agent{} {};

https::https() : req_delay{std::chrono::milliseconds(100)}, user_agent{} {};

void https::add_url(std::string url) { urls.push_back(std::move(url)); }

void https::reset_urls() { urls.clear(); }

void https::set_req_delay(size_t ms) {
        req_delay = std::chrono::milliseconds(ms);
}

std::vector<std::future<std::string>> https::extract() {
        if (!urls.size()) {
                std::cerr << "add sources to extract from" << std::endl
                          << "with extractor.add_url(std::string &url) method"
                          << std::endl;
                exit(1);
        }

        std::vector<std::future<std::string>> responses{};
        responses.reserve(urls.size());

        for (auto url : urls) {
                // std::this_thread::sleep_for(req_delay);

                responses.push_back(std::async(std::launch::async, make_req,
                                               url, user_agent, req_delay));
        }

        return responses;
}

} // namespace extractor
} // namespace ETL
