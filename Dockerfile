FROM debian:latest

WORKDIR /usr/server/homeserver

COPY . .

RUN ./bootstrap-debian.sh