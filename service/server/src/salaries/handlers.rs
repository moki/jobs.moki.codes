use std::collections::HashMap;

use actix_web::{web, HttpResponse, Responder};
use futures::stream::StreamExt;
use serde::Serialize;

use crate::stats;
use crate::Context;
use crate::Result;

const QUERY: &str = "select arrayJoin(skills), salary_avg, salary_currency from jobs_moki_codes.jobs where salary_avg is not null and salary_currency is not null and notEmpty(skills) and (salary_currency = 'RUR' or salary_currency = 'KZT' or salary_currency = 'EUR' or salary_currency = 'USD')";

pub async fn quartiles_and_counts(ctx: web::Data<Context>) -> impl Responder {
    let data = match retrieve(ctx).await {
        Ok(data) => data,
        Err(error) => {
            println!("{}", error);

            return HttpResponse::NotFound().body("404");
        }
    };

    match transform_data(data.as_slice()).await {
        Ok(data) => HttpResponse::Ok().json(data),
        Err(error) => {
            println!("{}", error);

            return HttpResponse::BadRequest().body("400");
        }
    }
}

const RUB: f64 = 0.014;
const KZT: f64 = 0.0024;
const EUR: f64 = 1.19;

#[derive(Debug, Serialize)]
struct SkillData {
    name: String,
    quartiles: stats::Quartiles,
    salary_count: Vec<(u64, usize)>,
    outliers_low: Vec<u64>,
    outliers_up: Vec<u64>,
    sample_size: usize,
}

async fn transform_data(data: &[(String, String, u64)]) -> Result<Vec<SkillData>> {
    let mut skill_salaries = HashMap::<&str, Vec<u64>>::new();

    for (skill, currency, salary) in data {
        let entry = skill_salaries.entry(skill).or_insert(Vec::new());

        let converted = match currency.as_ref() {
            "RUR" => *salary as f64 * RUB,
            "EUR" => *salary as f64 * EUR,
            "KZT" => *salary as f64 * KZT,
            "USD" => *salary as f64,
            _ => return Err("failed convert")?,
        };

        entry.push(converted.round() as u64);
    }

    skill_salaries.retain(|_, v: &mut Vec<u64>| v.len() > 90);

    let mut result: Vec<SkillData> = Vec::new();

    for (skill, salaries) in &mut skill_salaries {
        salaries.sort();

        let quartiles = stats::calculate_quartiles(&salaries)?;

        let sample_size = salaries.len();

        let mut salary_count = HashMap::<u64, usize>::new();

        for salary in salaries.iter() {
            let entry = salary_count.entry(*salary).or_insert(0);

            *entry += 1;
        }

        let salary_count: Vec<(u64, usize)> = salary_count.into_iter().collect();

        let mut outliers_low: Vec<u64> = Vec::new();
        let mut outliers_up: Vec<u64> = Vec::new();

        for salary in salaries.iter() {
            if (*salary as f64) > quartiles.upper_fence {
                outliers_up.push(*salary);
            }

            if (*salary as f64) < quartiles.first {
                outliers_low.push(*salary);
            }
        }

        result.push(SkillData {
            name: (*skill).into(),
            quartiles,
            salary_count,
            outliers_low,
            outliers_up,
            sample_size,
        });
    }

    result.sort_by(|a, b| b.sample_size.cmp(&a.sample_size));

    Ok(result)
}

async fn retrieve(ctx: web::Data<Context>) -> Result<Vec<(String, String, u64)>> {
    let mut client = ctx.pool.get_handle().await?;

    let mut result = Vec::new();

    let mut stream = client.query(QUERY).stream();

    while let Some(row) = stream.next().await {
        let row = row?;

        let skill: String = row.get(0)?;

        if let Some::<String>(currency) = row.get(2)? {
            if let Some::<u64>(salary_avg) = row.get(1)? {
                result.push((skill, currency, salary_avg));
            }
        }
    }

    Ok(result)
}
