use std::error::Error;
use std::fs::File;
use std::path::Path;

use serde::Deserialize;
use serde_yaml;

// Config - hh pipeline configuration options
//
// filename     - specify filename where to persist extracted data
// url          - url to extract data from
// query_params - Vector of tuples (key, value) allowing to specify
//                query parameters
// days         - for how long to go back extractacting data in days
// user_agent   - user-agent for the http extractor to use
#[derive(Debug, Deserialize)]
pub struct Config {
    pub(crate) filename: String,
    pub(crate) url: String,
    pub(crate) query_params: Vec<(String, String)>,
    pub(crate) days: u16,
    pub(crate) user_agent: String,
}

impl Config {
    // from_file - reads and creates configuration from
    //           - yaml configuration file specified by path
    pub fn from_file<P>(path: P) -> Result<Self, Box<dyn Error>>
    where
        P: AsRef<Path>,
    {
        let file = File::open(path.as_ref())?;

        let config: Config = serde_yaml::from_reader(&file)?;

        Ok(config)
    }
}
