FROM debian:10.10
LABEL maintainer=idi0tn3

# configurable version number
ARG BOOST_VER=1_73_0
RUN mv /etc/apt/sources.list /etc/apt/sources.list.bak && \
mkdir -p /usr/local/boost

ADD ./sources.list /etc/apt/sources.list
ADD ./boost_${BOOST_VER}.tar.gz /tmp/
ADD ./websocketpp /tmp/websocketpp

VOLUME ["/DataBackup"]

RUN apt-get update && \
apt-get install -y build-essential libssl-dev gcc g++ libjsoncpp-dev cmake

# compile boost
RUN cd /tmp/boost_${BOOST_VER} && \
./bootstrap.sh --prefix=/usr/local && \
./b2 install && \
ldconfig

# compile websocket
RUN cd /tmp/websocketpp && \
mkdir build && \
cd build && \
cmake .. && \
make && \
make install

# front-end
RUN apt-get install -y nodejs npm