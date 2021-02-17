#pragma once

#include "https.h"
#include <string>

namespace ETL {
namespace extractor {

class telegram : public https {
    public:
        using https::https;
        telegram();

    private:
};

telegram::telegram() {}

} // namespace extractor
} // namespace ETL
