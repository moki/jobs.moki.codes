#pragma once

#include "https.h"
#include <string>

namespace ETL {
namespace extractor {

class hh : public https {
    public:
        using https::https;
        hh();

    private:
};

hh::hh() {}

} // namespace extractor
} // namespace ETL
