FROM debian:bookworm

RUN apt update && \
    apt-get install -y gcc make libssl-dev

WORKDIR /capy/
