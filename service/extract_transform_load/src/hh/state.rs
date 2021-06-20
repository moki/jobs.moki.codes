#![allow(dead_code)]

use std::error::Error;
use std::fs::remove_file;
use std::fs::File;
use std::path::PathBuf;

use chrono::prelude::*;
use serde::{Deserialize, Serialize};
use serde_yaml;

// State - hh pipeline state
//
// start_from   - the date in UTC, tells service to continue extraction
//              - of the data from that point in time.
// path         - fs path where state gets persisted to and restored from.
#[derive(Serialize, Deserialize, Debug)]
pub struct State {
    pub(crate) start_from: DateTime<Utc>,
    path: PathBuf,
}

impl State {
    // new - creates new or restores previously held state from fs
    pub fn new(path: PathBuf) -> Self {
        State {
            start_from: Utc::now(),
            path: path,
        }

        /*
        match state.restore() {
            Ok(state) => state,
            _ => state,
        }
        */
    }

    // restore - restores state from the filesystem
    pub fn restore(&self) -> Result<Self, Box<dyn Error>> {
        let file = File::open(&self.path)?;

        let state: State = serde_yaml::from_reader(&file)?;

        Ok(state)
    }

    // persist - persists state to the filesystem
    pub fn persist(&self) -> Result<(), Box<dyn Error>> {
        let file = File::create(&self.path)?;

        serde_yaml::to_writer(file, self)?;

        Ok(())
    }

    // clean - clean up state from the filesystem
    pub fn clean(&self) -> Result<(), Box<dyn Error>> {
        remove_file(&self.path)?;

        Ok(())
    }
}
