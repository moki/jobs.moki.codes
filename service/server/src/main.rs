extern crate server;

use std::path::Path;

use actix_files::Files;
use actix_files::NamedFile;
use actix_web::{middleware, web, App, HttpServer, Result as ActixResult};
use clickhouse_rs::Pool;

use server::service::Config;
use server::skills::handlers;
use server::Context;
use server::Result;

async fn index(ctx: web::Data<Context>) -> ActixResult<NamedFile> {
    let i = format!(
        "{}",
        Path::new(&ctx.static_dir).join("index.html").display()
    );

    Ok(NamedFile::open(i)?)
}

#[actix_web::main]
async fn main() -> Result<(), std::io::Error> {
    let server_config = Config::from_file("config/config.yml").unwrap();

    let pool = Pool::new(server_config.clickhouse_uri);

    let static_dir = server_config.static_dir;
    let static_path = server_config.static_path;

    println!("binding to: {}:{}", server_config.host, server_config.port);

    HttpServer::new(move || {
        App::new()
            .wrap(middleware::Compress::default())
            .wrap(middleware::DefaultHeaders::new().header("Access-Control-Allow-Origin", "*"))
            .data(Context {
                pool: pool.clone(),
                static_dir: static_dir.clone(),
            })
            .service(web::scope("/api").service(
                web::scope("/skills").route("/dominance", web::get().to(handlers::skills)),
            ))
            .service(Files::new(&static_path, static_dir.clone()).prefer_utf8(true))
            .route("/", web::get().to(index))
    })
    .bind(format!("{}:{}", server_config.host, server_config.port))?
    .run()
    .await
}
