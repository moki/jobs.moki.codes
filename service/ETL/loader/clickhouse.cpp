#include "clickhouse.h"

namespace ETL {
namespace loader {

clickhouse::clickhouse(std::string &host, std::string &schema) {
        client = std::make_unique<Client>(ClientOptions().SetHost("localhost"));

        trim(schema);

        if (!schema.length())
                throw;

        if (schema[schema.length() - 1] == ';')
                schema = schema.substr(0, schema.length() - 1);

        auto statements = split(schema, ';');

        for (auto &statement : statements)
                Execute(statement);
}

clickhouse::~clickhouse() {}

std::vector<std::future<std::string>>
clickhouse::load(std::vector<std::future<std::string>> input) {
        std::vector<std::future<std::string>> loaded{};

        for (auto &future : input) {

                std::cout << "loading entry:" << std::endl;

                auto entry = future.get();

                std::cout << entry << std::endl;

                loaded.push_back(std::async(std::launch::async, [=]() {
                        return std::move(entry);
                }));
        }

        return std::move(loaded);
}

void clickhouse::Execute(std::string &statement) {
        if (!statement.length())
                return;

        std::cout << "executing: " << statement << std::endl;

        Query query{statement};

        client->Execute(query);
}

} // namespace loader
} // namespace ETL
