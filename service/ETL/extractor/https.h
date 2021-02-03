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
#include "Poco/URI.h"

namespace ETL {
namespace extractor {

using namespace std::string_literals;

class https : public virtual base {
        inline static auto make_req(std::string url) {
                std::string response{};
                auto uri = Poco::URI{url};

                Poco::Net::HTTPSClientSession session(uri.getHost(),
                                                      uri.getPort());
                Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
                                           uri.getPathAndQuery(),
                                           Poco::Net::HTTPMessage::HTTP_1_1);

                std::ostream &ostr = session.sendRequest(req);
                Poco::Net::HTTPResponse res;

                std::istream &rs = session.receiveResponse(res);
                Poco::StreamCopier::copyToString(rs, response);

                return std::move(response);
        };

    public:
        https(size_t delay);
        https();

        std::vector<std::future<std::string>> extract() override;
        void add_url(std::string &url);
        void reset_urls();
        void set_req_delay(size_t ms);

    private:
        std::chrono::milliseconds req_delay;
        std::vector<std::string> urls;
};

void https::add_url(std::string &url) { urls.push_back(std::move(url)); }
void https::reset_urls() { urls.clear(); }
void https::set_req_delay(size_t ms) {
        req_delay = std::chrono::milliseconds(ms);
}

https::https(size_t delay) : req_delay{std::chrono::milliseconds(delay)} {};
https::https() : req_delay{std::chrono::milliseconds(100)} {};

std::vector<std::future<std::string>> https::extract() {
        if (!urls.size()) {
                std::cerr << "add sources to extract from" << std::endl
                          << "with extractor.add_url(std::string &url) method"
                          << std::endl;
                exit(1);
        }

        std::vector<std::future<std::string>> responses{};
        responses.reserve(urls.size());

        // size_t i = 0;
        for (auto &url : urls) {
                // if (!(i++ % 10))
                std::this_thread::sleep_for(req_delay);

                responses.push_back(
                        std::async(std::launch::async, make_req, url));
        }

        return std::move(responses);
}

} // namespace extractor
} // namespace ETL
