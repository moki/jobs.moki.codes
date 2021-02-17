# Extract Transform Load

## Dependencies

* Clang
* CMake
* OpenSSL
* Poco
* simdjson(comes with the sources, lib/simdjson)

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
