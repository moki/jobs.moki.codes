extern crate chrono;
extern crate chrono_tz;
extern crate extract_transform_load;

use extract_transform_load::service;
use std::env;

const INTERNALS_DIR: &str = "internals";
const CONFIG_DIR: &str = "config";
const DATA_DIR: &str = "data";

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let arguments: Vec<String> = env::args().collect();

    let (internals, configuration, data): (&str, &str, &str) = match arguments.len() {
        3 => (&arguments[0], &arguments[1], &arguments[2]),
        _ => (INTERNALS_DIR, CONFIG_DIR, DATA_DIR),
    };

    let config = service::Config::new(internals, configuration, data);

    let service = service::Service::new(config);

    service.run().await?;

    Ok(())
}
