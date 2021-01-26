#pragma once

#include "https.h"
#include <string>

namespace ETL {
namespace extractor {

class moikrug : public https {
    public:
        using https::https;
        moikrug();

    private:
};

moikrug::moikrug() {}

} // namespace extractor
} // namespace ETL
