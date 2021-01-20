#pragma once

#include <future>
#include <iostream>

namespace ETL {

template <typename E, typename T, typename L> class ETL {
        ETL &operator=(const ETL &other) {}
        ETL(const ETL &other) {}

    public:
        ETL(E &extractor, T &transformer, L &loader);

        std::future<void> run();

        E &extractor;
        T &transformer;
        L &loader;
};

template <typename E, typename T, typename L>
ETL<E, T, L>::ETL(E &extractor, T &transformer, L &loader)
        : extractor(extractor), transformer(transformer), loader(loader){};

template <typename E, typename T, typename L>
std::future<void> ETL<E, T, L>::run() {
        return std::async(std::launch::async, [&]() {
                auto data = extractor.extract().get();
                auto transformed = transformer.transform(data);
                loader.load(transformed.get());
        });
}

} // namespace ETL
