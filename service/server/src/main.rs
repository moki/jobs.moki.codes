extern crate server;

use actix_web::{middleware, web, App, HttpServer};
use clickhouse_rs::Pool;

use server::service::Config;
use server::skills::handlers;
use server::Result;

#[actix_web::main]
async fn main() -> Result<(), std::io::Error> {
    let server_config = Config::from_file("config/config.yml").unwrap();

    let pool = Pool::new(server_config.clickhouse_uri);

    HttpServer::new(move || {
        App::new()
            .wrap(middleware::Compress::default())
            .data(pool.clone())
            .service(web::scope("/api").route("/skills", web::get().to(handlers::skills)))
    })
    .bind(format!("{}:{}", server_config.host, server_config.port))?
    .run()
    .await
}
