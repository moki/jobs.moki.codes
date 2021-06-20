from ekidd/rust-musl-builder:latest as builder

add --chown=rust:rust ./Cargo.toml ./Cargo.toml
add --chown=rust:rust ./src ./src

run cargo build --release

from alpine:latest

run apk --no-cache add ca-certificates

add ./config ./config

COPY --from=builder \
    /home/rust/src/target/x86_64-unknown-linux-musl/release/extract_transform_load \
    /usr/local/bin/

CMD /usr/local/bin/extract_transform_load
