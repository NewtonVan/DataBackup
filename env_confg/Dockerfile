FROM debian:10.10
MAINTAINER idi0tn3

# configurable version number
ARG BOOST_VER=1_73_0
ARG CMAKE_VER=3.20.5
RUN mv /etc/apt/sources.list /etc/apt/sources.list.bak && \
mkdir -p /usr/local/boost

ADD ./sources.list /etc/apt/sources.list
ADD ./boost_${BOOST_VER}.tar.gz /tmp/
ADD ./cmake-${CMAKE_VER}.tar.gz /tmp/
ADD ./websocketpp /tmp/websocketpp

VOLUME ["/DataBackup"]

RUN apt-get update && \
apt-get install -y build-essential libssl-dev gcc g++ libjsoncpp-dev && \
apt-get install -y nodejs npm

# compile cmake
RUN cd /tmp/cmake-${CMAKE_VER} && \
./bootstrap && \
make && \
make install && \
rm -fr /tmp/cmake-${CMAKE_VER}

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

# compile open-ssl
ARG OPENSSL_VER=3.0.0
ADD ./openssl-${OPENSSL_VER}.tar.gz /tmp/
RUN cd /tmp/openssl-${OPENSSL_VER} && \
./Configure && \
make && \
make install