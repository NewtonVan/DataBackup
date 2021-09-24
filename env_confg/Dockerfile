FROM debian:10.10
MAINTAINER idi0tn3


RUN mv /etc/apt/sources.list /etc/apt/sources.list.bak
ADD ./sources.list /etc/apt/sources.list

VOLUME ["/DataBackup"]

RUN apt update && \
apt install -y build-essential libssl-dev gcc g++ libjsoncpp-dev