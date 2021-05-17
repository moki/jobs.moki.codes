use super::hh;
pub use config::Config;
use std::fmt::Debug;
use std::fs;
use std::path::Path;

const HH_CONFIG: &str = "hh.yml";

pub(crate) mod config;

#[derive(Debug)]
pub struct Service {
    config: Config,
}

impl Service {
    pub fn new(config: Config) -> Self {
        Service { config }
    }

    pub async fn run(&self) -> Result<(), Box<dyn std::error::Error>> {
        fs::create_dir_all(&self.config.internals)?;
        fs::create_dir_all(&self.config.data)?;
        fs::create_dir_all(&self.config.state)?;

        let hh_config_filename = Path::new(&self.config.config).join(HH_CONFIG);

        let hh_config = hh::Config::from_file(hh_config_filename)?;

        let mut hh = hh::Service::new(hh_config, &self.config)?;

        hh.run().await?;

        Ok(())
    }
}
