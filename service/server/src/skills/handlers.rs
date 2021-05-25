use std::collections::BTreeMap;
use std::collections::HashMap;
use std::iter::FromIterator;

use actix_web::{web, HttpResponse, Responder};
use chrono::{DateTime, Duration, Utc};
use clickhouse_rs::Pool;
use futures::stream::StreamExt;
use serde::{Deserialize, Serialize};

use crate::Result;

#[derive(Deserialize)]
pub struct Query {
    from: Option<String>,
    to: Option<String>,
}

#[derive(Debug, Serialize, Deserialize)]
struct SkillIRData {
    date_occurences: BTreeMap<u32, u64>,
    total_occurences: u64,
}

#[derive(Debug, Serialize, Deserialize)]
struct SkillColumnarData {
    dates: Vec<u32>,
    occurences: Vec<u64>,
    total_occurences: u64,
}

async fn parse_query(q: &web::Query<Query>) -> (String, String) {
    let from = match &q.from {
        Some(from) => match DateTime::parse_from_rfc3339(from) {
            Ok(from) => from.format("%Y%m%d").to_string(),
            _ => (Utc::now() - Duration::days(30))
                .format("%Y%m%d")
                .to_string(),
        },
        _ => (Utc::now() - Duration::days(30))
            .format("%Y%m%d")
            .to_string(),
    };

    let to = match &q.to {
        Some(to) => match DateTime::parse_from_rfc3339(to) {
            Ok(to) => to.format("%Y%m%d").to_string(),
            _ => Utc::now().format("%Y%m%d").to_string(),
        },
        _ => Utc::now().format("%Y%m%d").to_string(),
    };

    return (from, to);
}

async fn retrieve_data<T>(pool: web::Data<Pool>, from: T, to: T) -> Result<Vec<(u32, Vec<String>)>>
where
    T: AsRef<str> + std::fmt::Display,
{
    let mut client = pool.get_handle().await?;

    let query = format!("select toYYYYMMDD(created), flatten(groupArray(skills)) from jobs_moki_codes.jobs where toYYYYMMDD(created) between {} and {} group by toYYYYMMDD(created) order by toYYYYMMDD(created) asc", from, to);

    let mut date_skills = Vec::new();

    let mut stream = client.query(query).stream();

    while let Some(row) = stream.next().await {
        let row = row?;

        let date: u32 = row.get(0)?;

        let skills: Vec<String> = row.get(1)?;

        date_skills.push((date, skills));
    }

    Ok(date_skills)
}

// TODO: cache data
async fn aggregate_data<T>(
    pool: web::Data<Pool>,
    from: T,
    to: T,
) -> Result<HashMap<String, SkillColumnarData>>
where
    T: AsRef<str> + std::fmt::Display,
{
    let data = retrieve_data(pool, from, to).await?;

    let mut skill_skill_data = HashMap::new();

    for (date, skills) in &data {
        for skill in skills {
            let skill_entry = skill_skill_data.entry(skill).or_insert(SkillIRData {
                date_occurences: BTreeMap::new(),
                total_occurences: 0,
            });

            let date_entry = skill_entry.date_occurences.entry(*date).or_insert(0);

            *date_entry += 1;

            skill_entry.total_occurences += 1;
        }
    }

    let mut skill_columnar_data = HashMap::new();

    for (name, value) in &skill_skill_data {
        skill_columnar_data
            .entry((*name).to_owned())
            .or_insert(SkillColumnarData {
                dates: Vec::from_iter(value.date_occurences.keys().map(|&k| k)),
                occurences: Vec::from_iter(value.date_occurences.values().map(|&v| v)),
                total_occurences: value.total_occurences,
            });
    }

    Ok(skill_columnar_data)
}

pub async fn skills(pool: web::Data<Pool>, q: web::Query<Query>) -> impl Responder {
    let (from, to) = parse_query(&q).await;

    match aggregate_data(pool, from, to).await {
        Ok(data) => HttpResponse::Ok().json(data),
        _ => HttpResponse::BadRequest().body("bye world!"),
    }
}
