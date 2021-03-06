use std::path::{Path, PathBuf};

const STATE_DIR: &str = "state";

// Config - service configuration options
//
// internals - specify directory for service internal usage
// config    - specify directory to read each service configuration from
// data      - specify directory to store extracted data
// state     - hardcoded state directory
#[derive(Debug)]
pub struct Config {
    pub internals: PathBuf,
    pub config: PathBuf,
    pub data: PathBuf,
    pub state: PathBuf,
}

impl Config {
    pub fn new<P: AsRef<Path>>(internals: P, config: P, data: P) -> Self {
        Config {
            internals: PathBuf::from(internals.as_ref()),
            config: PathBuf::from(config.as_ref()),
            data: PathBuf::from(data.as_ref()),
            state: PathBuf::from(internals.as_ref().join(STATE_DIR)),
        }
    }
}
