use super::transformer::Job;

use chrono::DateTime;
use chrono_tz::Tz;
use chrono_tz::Tz::UTC;
use std::error::Error;

use clickhouse_rs::{types::Block, Pool};

const CREATE_DB_STMT: &str = "create database if not exists jobs_moki_codes";

const USE_DB_STMT: &str = "use jobs_moki_codes";

const DEDUPLICATE_STMT: &str = "optimize table jobs_moki_codes.jobs final DEDUPLICATE";

const CREATE_TABLE_STMT: &str = "create table if not exists jobs_moki_codes.jobs (
        local_id                String,
        source                  String,
        title                   String,
        area                    String,
        salary_avg              Nullable(UInt64),
        salary_currency         Nullable(String),
        created                 DateTime('UTC'),
        skills                  Array(String),
        remote                  UInt8,
        experience              UInt8,
        specialization_ids      Array(String),
        specialization_names    Array(String)
) Engine=MergeTree order by created partition by toYYYYMM(created)";

pub(crate) struct ClickHouse {
    pool: Pool,
}

impl ClickHouse {
    pub(crate) fn new<T>(db_uri: T) -> Result<Self, Box<dyn Error>>
    where
        T: AsRef<str>,
    {
        Ok(Self {
            pool: Pool::new(db_uri.as_ref().to_string()),
        })
    }

    pub(crate) async fn initialize(&self) -> Result<(), Box<dyn Error>> {
        let mut client = self.pool.get_handle().await?;

        client.execute(CREATE_DB_STMT).await?;

        client.execute(USE_DB_STMT).await?;

        client.execute(CREATE_TABLE_STMT).await?;

        Ok(())
    }

    pub(crate) async fn store(&self, jobs: &mut Vec<Job>) -> Result<(), Box<dyn Error>> {
        let size = jobs.len();

        // TODO: factor out to the "to columnar" transform function.
        // inside the transformer module

        let poss_title_skills = [
            "junior",
            "middle",
            "senior",
            "младший",
            "старший",
            "ведущий",
            "lead",
            "front",
            "back",
            "full",
        ];

        let mut ids: Vec<String> = Vec::with_capacity(size);
        let mut titles: Vec<String> = Vec::with_capacity(size);
        let mut areas: Vec<String> = Vec::with_capacity(size);
        let mut salary_avg: Vec<Option<u64>> = Vec::with_capacity(size);
        let mut salary_curr: Vec<Option<String>> = Vec::with_capacity(size);
        let mut dates: Vec<DateTime<Tz>> = Vec::with_capacity(size);
        let mut skills: Vec<Vec<String>> = Vec::with_capacity(size);
        let mut remotes: Vec<u8> = Vec::with_capacity(size);
        let mut exps: Vec<u8> = Vec::with_capacity(size);
        let mut spec_ids: Vec<Vec<String>> = Vec::with_capacity(size);
        let mut spec_names: Vec<Vec<String>> = Vec::with_capacity(size);
        let sources: Vec<String> = (0..jobs.len()).into_iter().map(|_| "hh".into()).collect();

        for job in jobs {
            ids.push(job.id.to_owned());
            titles.push(job.title.to_owned());

            for t in job.title.split(' ') {
                let ts = t.to_string();

                if poss_title_skills.contains(&t) && !job.skills.contains(&ts) {
                    job.skills.push(ts);
                }
            }

            job.skills.sort();

            job.skills.dedup();

            areas.push(job.area.to_owned());
            salary_avg.push(job.salary.as_ref().map(|s| s.avg));
            salary_curr.push(job.salary.as_ref().map(|s| s.currency.to_owned()));
            dates.push(job.created.with_timezone(&UTC));
            skills.push(job.skills.to_owned());
            remotes.push(if job.remote { 1 } else { 0 });
            exps.push(job.experience);
            spec_ids.push(
                job.specializations
                    .iter()
                    .map(|s| s[0].to_owned())
                    .collect(),
            );
            spec_names.push(
                job.specializations
                    .iter()
                    .map(|s| s[1].to_owned())
                    .collect(),
            );
        }

        let block = Block::new()
            .column("local_id", ids)
            .column("source", sources)
            .column("title", titles)
            .column("area", areas)
            .column("salary_avg", salary_avg)
            .column("salary_currency", salary_curr)
            .column("created", dates)
            .column("skills", skills)
            .column("remote", remotes)
            .column("experience", exps)
            .column("specialization_ids", spec_ids)
            .column("specialization_names", spec_names);

        let mut client = self.pool.get_handle().await?;

        client.insert("jobs_moki_codes.jobs", block).await?;

        client.execute(DEDUPLICATE_STMT).await?;

        Ok(())
    }
}
