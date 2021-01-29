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

                std::cout << res.getStatus() << std::endl;

                std::istream &rs = session.receiveResponse(res);
                Poco::StreamCopier::copyToString(rs, response);

                return std::move(response);
        };

        inline static auto batch_reqs = [](auto self) {
                std::vector<std::future<std::string>> responses{};
                responses.reserve(self->urls.size());

                size_t i = 0;
                for (auto &url : self->urls) {
                        std::cout << "making req: " << i << std::endl;
                        responses.push_back(std::async(std::launch::async,
                                                       self->make_req, url));
                        if (!(i % 10))
                                std::this_thread::sleep_for(self->req_delay);

                        i++;
                }

                auto response =
                        std::async([rs = std::move(responses)]() mutable {
                                std::vector<std::string> responses{};
                                responses.reserve(rs.size());

                                size_t i = 0;
                                for (auto &r : rs) {
                                        responses.push_back(r.get());
                                        std::cout << "joined req: " << i++
                                                  << std::endl;
                                }

                                return responses;
                        }).get();

                std::cout << "joined all requests" << std::endl;

                /*
                std::string response_str{};

                response_str += *(response.begin());

                for (auto it = response.begin() + 1; it != response.end();
                     it++) {
                        response_str += '\n' + *it;
                }

                return response_str;
                */

                return self->reducer(response);
        };

    public:
        https(size_t delay);
        https();

        std::future<std::string> extract() override;
        void add_url(std::string &url);
        void reset_urls();
        void set_reducer(auto fn);
        void set_req_delay(size_t ms);

    private:
        std::chrono::milliseconds req_delay;
        std::function<std::string(std::vector<std::string>)> reducer;
        std::vector<std::string> urls;
};

void https::set_reducer(auto fn) { reducer = fn; }
void https::add_url(std::string &url) { urls.push_back(std::move(url)); }
void https::reset_urls() { urls.clear(); }
void https::set_req_delay(size_t ms) {
        req_delay = std::chrono::milliseconds(ms);
}

https::https(size_t delay) : req_delay{std::chrono::milliseconds(delay)} {};
https::https() : req_delay{std::chrono::milliseconds(250)} {};

std::future<std::string> https::extract() {
        if (!urls.size()) {
                std::cerr << "add sources to extract from" << std::endl
                          << "with extractor.add_url(std::string &url) method"
                          << std::endl;
                exit(1);
        }

        if (!reducer) {
                std::cerr << "set reducer to accumulate responses" << std::endl
                          << "with "
                             "extractor.set_reducer(std::function<std::string("
                             "std::vector<std::string>)>) method"
                          << std::endl;
                exit(1);
        }

        return std::async(std::launch::async, batch_reqs, this);
}

} // namespace extractor
} // namespace ETL
