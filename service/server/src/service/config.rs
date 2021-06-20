use std::error::Error;
use std::fs::File;
use std::path::Path;

use serde::Deserialize;
use serde_yaml;
// Config - server configuration options
//
// clickhouse_uri - clickhouse database connection uri
// static_dir     - static files directory
// static_path    - serve static files from path
// host           - server host
// port           - server port
#[derive(Debug, Deserialize)]
pub struct Config {
    pub clickhouse_uri: String,
    pub static_dir: String,
    pub static_path: String,
    pub host: String,
    pub port: u16,
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
