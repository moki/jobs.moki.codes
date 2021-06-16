use std::collections::BTreeMap;
use std::collections::HashMap;
use std::iter::FromIterator;

use actix_web::{web, HttpResponse, Responder};
//use chrono::{DateTime, Duration, Utc};
use chrono::{Duration, Utc};
use clickhouse_rs::Pool;
use futures::stream::StreamExt;
use serde::{Deserialize, Serialize};

use crate::Result;

/*
#[derive(Deserialize)]
pub struct Query {
    from: Option<String>,
    to: Option<String>,
}
*/

#[derive(Debug, Serialize, Deserialize)]
struct SkillIRData {
    date_occurences: BTreeMap<u32, u64>,
    total_occurences: u64,
}

#[derive(Debug, Serialize, Deserialize)]
struct SkillColumnarData {
    name: String,
    dates: Vec<u32>,
    occurences: Vec<u64>,
    total_occurences: u64,
}

/*
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
*/

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
async fn aggregate_data<T>(pool: web::Data<Pool>, from: T, to: T) -> Result<Vec<SkillColumnarData>>
where
    T: AsRef<str> + std::fmt::Display,
{
    let data = retrieve_data(pool, from, to).await?;

    let mut skill_skill_data = HashMap::new();

    let dates = BTreeMap::from_iter(data.clone().into_iter().map(|(d, _)| (d, 0)).into_iter());

    for (date, skills) in &data {
        for skill in skills {
            let skill_entry = skill_skill_data.entry(skill).or_insert(SkillIRData {
                date_occurences: dates.clone(),
                total_occurences: 0,
            });

            if let Some(date_entry) = skill_entry.date_occurences.get_mut(date) {
                *date_entry += 1;
                skill_entry.total_occurences += 1;
            }
        }
    }

    let mut skill_columnar_data = Vec::with_capacity(skill_skill_data.len());

    for (name, value) in &skill_skill_data {
        skill_columnar_data.push(SkillColumnarData {
            name: (*name).to_owned(),
            dates: Vec::from_iter(value.date_occurences.keys().map(|&k| k)),
            occurences: Vec::from_iter(value.date_occurences.values().map(|&v| v)),
            total_occurences: value.total_occurences,
        });
    }

    skill_columnar_data.sort_by(|a, b| b.total_occurences.cmp(&a.total_occurences));

    Ok(skill_columnar_data)
}

//pub async fn skills(pool: web::Data<Pool>, q: web::Query<Query>) -> impl Responder {
// temp: serve only last month worth of data
pub async fn skills(pool: web::Data<Pool>) -> impl Responder {
    //let (from, to) = parse_query(&q).await;
    let (from, to) = (
        (Utc::now() - Duration::days(30))
            .format("%Y%m%d")
            .to_string(),
        Utc::now().format("%Y%m%d").to_string(),
    );

    match aggregate_data(pool, from, to).await {
        Ok(data) => HttpResponse::Ok().json(data),
        _ => HttpResponse::BadRequest().body("400"),
    }
}
