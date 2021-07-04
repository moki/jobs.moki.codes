use std::collections::BTreeMap;
use std::collections::HashMap;
use std::iter::repeat;
use std::iter::FromIterator;

use actix_web::{web, HttpResponse, Responder};
use chrono::{Duration, Utc};
use serde::{Deserialize, Serialize};

use futures::stream::StreamExt;

use crate::Context;
use crate::Result;

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

async fn retrieve_data<T>(
    ctx: web::Data<Context>,
    from: T,
    to: T,
) -> Result<Vec<(u32, Vec<String>)>>
where
    T: AsRef<str> + std::fmt::Display,
{
    let mut client = ctx.pool.get_handle().await?;

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

#[derive(Debug, Serialize, Deserialize)]
struct SkillInfo {
    skill_occurences: HashMap<String, u64>,
    total_occurences: u64,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
struct SkillColumnar {
    name: String,
    dates: Vec<u32>,
    dominance: Vec<f32>,
    total_occurences: u64,
}

// TODO: cache data
async fn aggregate_data<T>(ctx: web::Data<Context>, from: T, to: T) -> Result<Vec<SkillColumnar>>
where
    T: AsRef<str> + std::fmt::Display,
{
    let data = retrieve_data(ctx, from, to).await?;

    let dates: Vec<u32> = data.clone().into_iter().map(|(d, _)| d).collect();

    let mut date_skill = BTreeMap::from_iter(
        data.clone()
            .into_iter()
            .map(|(d, _)| {
                (
                    d,
                    SkillInfo {
                        total_occurences: 0,
                        skill_occurences: HashMap::new(),
                    },
                )
            })
            .into_iter(),
    );

    for (date, skills) in &data {
        let date_entry = date_skill.get_mut(date).unwrap();

        for skill in skills {
            if let Some(skill_entry) = date_entry.skill_occurences.get_mut(skill) {
                *skill_entry += 1;
            } else {
                date_entry.skill_occurences.insert(skill.clone(), 1);
            }

            date_entry.total_occurences += 1;
        }
    }

    let mut skill_dominance: HashMap<String, SkillColumnar> = HashMap::new();

    let mut i = 0;

    for (_, date_entry) in &date_skill {
        for (name, occured) in &date_entry.skill_occurences {
            let skill_dominance_entry =
                skill_dominance
                    .entry(name.clone())
                    .or_insert(SkillColumnar {
                        name: name.clone(),
                        dates: dates.clone(),
                        dominance: repeat(0_f32).take(dates.len()).collect(),
                        total_occurences: 0,
                    });

            skill_dominance_entry.dominance[i] =
                100_f32 * ((*occured) as f64 / (date_entry.total_occurences) as f64) as f32;
            skill_dominance_entry.total_occurences += occured;
        }

        i = i + 1;
    }

    let mut skill_columnar = skill_dominance
        .values()
        .cloned()
        .collect::<Vec<SkillColumnar>>();

    skill_columnar.sort_by(|a, b| b.total_occurences.cmp(&a.total_occurences));

    Ok(skill_columnar)
}

// temp: serve only last month worth of data
pub async fn dominance(ctx: web::Data<Context>) -> impl Responder {
    let right = Utc::now() - Duration::days(1);
    let left = right - Duration::days(30);

    let left = left.format("%Y%m%d").to_string();
    let right = right.format("%Y%m%d").to_string();

    match aggregate_data(ctx, left, right).await {
        Ok(data) => HttpResponse::Ok().json(data),
        Err(error) => {
            println!("{}", error);

            return HttpResponse::BadRequest().body("400");
        }
    }
}
