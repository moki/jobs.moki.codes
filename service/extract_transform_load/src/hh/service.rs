use super::state::State;
use super::Config;
use crate::service;
use std::error;
use std::path::PathBuf;

const STATE_FILE_NAME: &str = "hh.yml";

#[derive(Debug)]
pub struct Service {
    state_path: PathBuf,
    data_path: PathBuf,
    source_url: String,
    query_params: Vec<(String, String)>,
    state: State,
}

impl Service {
    pub fn new(config: Config, common: &service::Config) -> Self {
        let state_path = PathBuf::from(&common.state).join(STATE_FILE_NAME);

        Service {
            state_path: state_path.to_owned(),
            data_path: PathBuf::from(&common.data).join(&config.filename),
            source_url: config.url,
            query_params: config.query_params,
            state: State::restore(&state_path).unwrap_or(State::new()),
        }
    }
    pub async fn run(&mut self) -> Result<(), Box<dyn error::Error>> {
        println!("{}", self.state.start_from);

        self.state.start_from = self.state.start_from + chrono::Duration::hours(1);

        self.state.persist(&self.state_path)?;

        Ok(())
    }
}
