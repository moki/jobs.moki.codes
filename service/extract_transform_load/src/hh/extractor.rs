use futures::future::join_all;
use reqwest::{Client, ClientBuilder, Method, Response};
use serde::Deserialize;
use std::error::Error;
use std::time::Duration;
use tokio::time;

const DELAY_LIMIT: Duration = Duration::from_micros(10_000_000);

pub(crate) struct Extractor {
    url: String,
    query_params: Vec<(String, String)>,
    client: Client,
    delay_limit: Duration,
}

pub(crate) struct Config {
    pub(crate) url: String,
    pub(crate) query_params: Vec<(String, String)>,
    pub(crate) user_agent: String,
    pub(crate) delay_limit: Option<Duration>,
}

#[derive(Deserialize, Debug)]
struct PagesResponse {
    pages: u32,
}

#[derive(Deserialize, Debug)]
struct UrlResponse {
    url: String,
}

#[derive(Deserialize, Debug)]
struct UrlsResponse {
    items: Vec<UrlResponse>,
}

#[derive(Deserialize, Debug)]
pub(crate) struct Salary {
    from: u64,
    to: u64,
    currency: String,
}

#[derive(Deserialize, Debug)]
pub(crate) struct Area {
    name: String,
}

#[derive(Deserialize, Debug)]
pub(crate) struct Schedule {
    id: String,
}

#[derive(Deserialize, Debug)]
pub(crate) struct Specialization {
    id: String,
    name: String,
}

#[derive(Deserialize, Debug)]
pub(crate) struct Skill {
    name: String,
}

#[derive(Deserialize, Debug)]
pub(crate) struct Experience {
    id: String,
}

#[derive(Deserialize, Debug)]
pub(crate) struct Job {
    #[serde(rename(deserialize = "name"))]
    pub(crate) title: String,
    pub(crate) id: String,
    pub(crate) salary: Salary,
    pub(crate) area: Area,
    pub(crate) schedule: Schedule,
    #[serde(rename(deserialize = "published_at"))]
    pub(crate) created: String,
    #[serde(rename(deserialize = "specializations"))]
    pub(crate) fields: Vec<Specialization>,
    #[serde(rename(deserialize = "key_skills"))]
    pub(crate) skills: Vec<Skill>,
    pub(crate) experience: Experience,
}

impl Extractor {
    pub(crate) fn new(config: Config) -> Result<Self, Box<dyn Error>> {
        let url = format!(
            "{}?{}={}&{}={}&{}={}",
            config.url,
            config.query_params[0].0,
            config.query_params[0].1,
            config.query_params[1].0,
            config.query_params[1].1,
            config.query_params[5].0,
            config.query_params[5].1,
        );

        Ok(Extractor {
            url: url,
            query_params: config.query_params,
            client: ClientBuilder::new()
                .user_agent(&config.user_agent)
                .build()?,
            delay_limit: config.delay_limit.unwrap_or(DELAY_LIMIT),
        })
    }

    async fn http_req<T>(&self, url: T, method: Method) -> Result<Response, Box<dyn Error>>
    where
        T: AsRef<str>,
    {
        let mut delay = Duration::from_micros(1);

        loop {
            let res = self.client.request(method.clone(), url.as_ref()).send();

            if let Ok(r) = res.await {
                if r.status().as_u16() == 200 {
                    return Ok(r);
                }
            }

            time::sleep(delay).await;

            if delay < self.delay_limit {
                delay *= 2;
            }
        }
    }

    pub(crate) async fn extract_in_timeframe<T>(&self, from: T, to: T) -> Vec<Job>
    where
        T: AsRef<str>,
    {
        let url = format!(
            "{}&{}={}&{}={}",
            self.url,
            self.query_params[3].0,
            from.as_ref(),
            self.query_params[4].0,
            to.as_ref(),
        );

        let pages = match self.http_req(&url, Method::GET).await {
            Ok(res) => match res.json::<PagesResponse>().await {
                Ok(body) => body.pages,
                _ => 0,
            },
            _ => 0,
        };

        let url_reqs = (0..pages).map(|p| {
            let url = format!("{}&page={}", &url, p);

            self.http_req(url, Method::GET)
        });

        let mut job_reqs = Vec::new();

        for r in join_all(url_reqs).await.into_iter() {
            if let Ok(res) = r {
                if let Ok(body) = res.json::<UrlsResponse>().await {
                    for i in body.items {
                        job_reqs.push(self.http_req(i.url, Method::GET));
                    }
                }
            }
        }

        let mut jobs = Vec::new();

        for r in join_all(job_reqs).await.into_iter() {
            if let Ok(res) = r {
                if let Ok(body) = res.json::<Job>().await {
                    jobs.push(body);
                }
            }
        }

        jobs
    }
}
