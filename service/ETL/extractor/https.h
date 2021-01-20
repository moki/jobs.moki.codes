#pragma once

#include "extractor.h"
#include <future>
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

                return response;
        };

        inline static auto batch_reqs(std::string url, size_t page_start,
                                      size_t pages_total) {
                std::vector<std::future<std::string>> responses{};
                responses.reserve(pages_total);
                size_t i = page_start;

                for (; i < pages_total; i++) {
                        responses.push_back(
                                std::async(std::launch::async, make_req,
                                           url + std::to_string(i)));
                        if (!(i % 10))
                                std::this_thread::sleep_for(
                                        std::chrono::milliseconds(200));
                }

                auto response =
                        std::async([rs = std::move(responses)]() mutable {
                                std::vector<std::string> responses{};
                                responses.reserve(rs.size());

                                for (auto &r : rs)
                                        responses.push_back(r.get());

                                return responses;
                        }).get();

                return std::accumulate(response.begin() + 1, response.end(),
                                       std::move("[" + response[0]),
                                       [](std::string a, std::string b) {
                                               return std::move(a) +
                                                      std::move(",") +
                                                      std::move(b);
                                       }) +
                       std::move("]");
        }

    public:
        https(std::string_view &&url, size_t start, size_t pages_total);
        std::future<std::string> extract() override;

        void set_pages_total(size_t);

    private:
        size_t page_start;
        size_t pages_total;

        std::string url;
};

https::https(std::string_view &&url, size_t page_start = 0,
             size_t pages_total = 1)
        : url(std::move(url)), page_start(page_start),
          pages_total(page_start + pages_total) {}

std::future<std::string> https::extract() {
        return std::async(std::launch::async, batch_reqs, url, page_start,
                          pages_total);
}

void https::set_pages_total(size_t pages) { pages_total = pages + page_start; }

} // namespace extractor
} // namespace ETL
