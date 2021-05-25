pub mod service;
pub mod skills;

pub type Result<T, E = Box<dyn std::error::Error>> = std::result::Result<T, E>;
