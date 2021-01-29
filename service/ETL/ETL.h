#pragma once

#include <future>
#include <iostream>

#include "extractor/extractor.h"
#include "loader/loader.h"
#include "transformer/transformer.h"

namespace ETL {

class ETL {
        ETL &operator=(const ETL &other) = delete;
        ETL(const ETL &other) = delete;

    public:
        ETL(extractor::base &extractor, transformer::base &transformer,
            loader::base &loader);

        std::future<void> run();

        void set_extractor(extractor::base &extractor);
        void set_transformer(transformer::base &transformer);
        void set_loader(loader::base &loader);

        std::reference_wrapper<extractor::base> extractor;
        std::reference_wrapper<transformer::base> transformer;
        std::reference_wrapper<loader::base> loader;
};

ETL::ETL(extractor::base &extractor, transformer::base &transformer,
         loader::base &loader)
        : extractor(std::ref(extractor)), transformer(std::ref(transformer)),
          loader(std::ref(loader)){};

std::future<void> ETL::run() {
        return std::async(std::launch::async, [&]() {
                std::cout << "extracting..." << std::endl;
                auto data = extractor.get().extract().get();
                std::cout << "transforming..." << std::endl;
                auto transformed = transformer.get().transform(data);
                std::cout << "loading..." << std::endl;
                loader.get().load(transformed.get()).get();
        });
}

void ETL::set_extractor(extractor::base &extractor) {
        this->extractor = std::ref(extractor);
}

void ETL::set_transformer(transformer::base &transformer) {
        this->transformer = std::ref(transformer);
}

void ETL::set_loader(loader::base &loader) { this->loader = std::ref(loader); }

} // namespace ETL
