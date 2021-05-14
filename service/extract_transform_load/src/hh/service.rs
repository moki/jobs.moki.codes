use super::extractor;
use super::state::State;
use super::Config;
use crate::service;

use std::error::Error;
use std::path::PathBuf;

use chrono::prelude::*;
use chrono::Duration;
use chrono_tz::Europe::Moscow;

const STATE_FILENAME: &str = "hh.yml";

pub struct Service {
    extractor: extractor::Extractor,
    state: State,
    days: i64,
}

impl Service {
    pub fn new(config: Config, common: &service::Config) -> Result<Self, Box<dyn Error>> {
        let state_path = PathBuf::from(&common.state).join(STATE_FILENAME);
        data_path = PathBuf::from(&common.data).join(&config.filename);

        let extractor_config = extractor::Config {
            url: config.url,
            query_params: config.query_params,
            user_agent: config.user_agent,
            delay_limit: None,
        };

        let extractor = extractor::Extractor::new(extractor_config)?;

        Ok(Service {
            state: State::new(state_path),
            extractor: extractor,
            days: config.days as i64,
        })
    }

    pub async fn run(&mut self) -> Result<(), Box<dyn Error>> {
        let time_from = self.state.start_from - Duration::days(self.days);

        for day in (0..self.days).rev() {
            let day_cursor = time_from + Duration::days(day);
            let hours = 6;

            for hour in 0..hours {
                let left = day_cursor + chrono::Duration::hours(hour);
                let left = Moscow.from_utc_datetime(&left.naive_utc());
                let left = left.format("%Y-%m-%dT%H:%M:%S").to_string();

                let right = day_cursor + chrono::Duration::hours(hour + 1);
                let right = Moscow.from_utc_datetime(&right.naive_utc());
                let right = right.format("%Y-%m-%dT%H:%M:%S").to_string();

                let jobs = self.extractor.extract_in_timeframe(&left, &right);
            }

            self.state.start_from = day_cursor - chrono::Duration::days(1);

            self.state.persist()?;
        }

        Ok(())
    }
}
