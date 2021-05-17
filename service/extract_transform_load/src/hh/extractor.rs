use super::transformer::*;

use std::error::Error;
use std::time::Duration;

use futures::future::join_all;
use reqwest::{Client, ClientBuilder, Method, Response};
use serde::Deserialize;
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
                .timeout(Duration::from_micros(5_000_000))
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
