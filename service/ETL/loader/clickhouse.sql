create database if not exists "jobs_moki_codes";
create table if not exists "jobs_moki_codes"."jobs" (
        local_id        UInt64          NOT NULL,
        date            Date            NOT NULL,
        title           String          NOT NULL,
        skills          Array(String)   NOT NULL,
        currency        String          NULL,
        salary_range    Array(UInt64)   NOT NULL,
        salary_avg      UInt64          materialized if (not empty(salary_range), divide(plus(salary_range[1], salary_range[2]), 2), 0),
        locations       Array(String)   NOT NULL,
        remote          UInt8           NOT NULL
) engine = MergeTree partition by toYYYYMM(date) order by date;
