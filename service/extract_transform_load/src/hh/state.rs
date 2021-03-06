use chrono::prelude::*;
use serde::{Deserialize, Serialize};
use serde_yaml;
use std::error;
use std::fs::File;
use std::path::Path;

// State - hh pipeline state
//
// start_from   - the date in UTC, tells service to continue extraction
//              - of the data from that point in time.
#[derive(Debug, Serialize, Deserialize)]
pub struct State {
    pub(crate) start_from: DateTime<Utc>,
}

impl State {
    pub fn new() -> Self {
        State {
            start_from: Utc::now(),
        }
    }

    // restore - restores state from filesystem
    pub fn restore<P>(path: P) -> Result<Self, Box<dyn error::Error>>
    where
        P: AsRef<Path>,
    {
        let file = File::open(path)?;

        let state: State = serde_yaml::from_reader(&file)?;

        Ok(state)
    }

    // persist - persists state to filesystem
    pub fn persist<P>(&self, path: P) -> Result<(), Box<dyn error::Error>>
    where
        P: AsRef<Path>,
    {
        let file = File::create(path)?;

        serde_yaml::to_writer(file, self)?;

        Ok(())
    }
}
