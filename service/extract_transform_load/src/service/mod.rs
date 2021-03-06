use super::hh;
pub use config::Config;
use std::fmt::Debug;
use std::fs;
use std::path::Path;

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

        let hh_config = Path::new(&self.config.config).join("hh.yml");
        let hh_config = hh::Config::from_file(hh_config)?;

        let mut hh = hh::Service::new(hh_config, &self.config);

        println!("wrapper service");
        println!("{:?}", self);
        println!("hh service");
        println!("{:?}", hh);

        hh.run().await?;

        Ok(())
    }
}