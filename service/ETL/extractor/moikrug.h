#pragma once

#include "https.h"
#include <string>

namespace ETL {
namespace extractor {

class moikrug : public https {
    public:
        using https::https;
        moikrug(std::string_view &&url);
    private:
};

moikrug::moikrug(std::string_view &&url) : https(std::move(url), 1, 1) {}

} // namespace extractor
} // namespace ETL
