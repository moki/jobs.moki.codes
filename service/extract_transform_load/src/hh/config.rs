use serde::Deserialize;
use serde_yaml;
use std::error;
use std::fs::File;
use std::path::Path;

// Config - hh pipeline configuration options
//
// filename     - specify filename where to persist extracted data
// url          - url to extract data from
// query_params - Vector of tuples (key, value) allowing to specify
//                query parameters
#[derive(Debug, Deserialize)]
pub struct Config {
    pub(crate) filename: String,
    pub(crate) url: String,
    pub(crate) query_params: Vec<(String, String)>,
}

impl Config {
    // from_file - reads and creates configuration from
    //           - yaml configuration file specified by path
    pub fn from_file<P>(path: P) -> Result<Self, Box<dyn error::Error>>
    where
        P: AsRef<Path>,
    {
        let file = File::open(path)?;

        let config: Config = serde_yaml::from_reader(&file)?;

        Ok(config)
    }
}
