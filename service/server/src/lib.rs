pub mod service;
pub mod skills;

use clickhouse_rs::Pool;

pub type Result<T, E = Box<dyn std::error::Error>> = std::result::Result<T, E>;

pub struct Context {
    pub pool: Pool,
    pub static_dir: String,
}
