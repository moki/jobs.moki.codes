#pragma once

#include "loader.h"

#include "../utility/split.h"
#include "../utility/trim.h"

#include <clickhouse/client.h>
#include <iostream>

namespace ETL {
namespace loader {

using namespace clickhouse;

class clickhouse : public virtual base {
    public:
        clickhouse(std::string &host, std::string &schema);
        ~clickhouse();

        std::vector<std::future<std::string>>
        load(std::vector<std::future<std::string>> input) override;

    private:
        std::unique_ptr<Client> client;

        void Execute(std::string &statement);
};

} // namespace loader
} // namespace ETL
