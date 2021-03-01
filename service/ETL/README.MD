# Extract Transform Load

## Dependencies

* Clang
* CMake
* [Clickhouse](https://github.com/ClickHouse/ClickHouse)

### Shared

* OpenSSL
* [Poco](https://github.com/pocoproject/poco)
* [clickhouse-cpp](https://github.com/ClickHouse/clickhouse-cpp)

### static

comes with sources

* [simdjson](https://github.com/simdjson/simdjson)
  * [lib/simdjson](lib/simdjson)
* [date](https://github.com/HowardHinnant/date)
  * [lib/date](lib/date)

ETL is a data pipeline.
Each component consists of extractor, transformer and loader.

## ETL's

* moikrug
* hh
* telegram profunctor

## folder structure

```
extractor/<source_name>
transformer/<transform_name>
loader/<data_lake_name>
```

## extractor

extractor - extracts data from source

### data sources

* hh(https)
* moikrug(https)
* telegram profunctor(https)

## transformer

transformer - transforms data to the appropriate form i.e cleansing

### transformers

* hh(html)
* moikrug(json)
* telegram profunctor(html)

## loader

loader - loads data into the data lake

### data lakes

* hh(fs)
* moikrug(fs)
* telegram profunctor(fs)
