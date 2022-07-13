FROM debian:bullseye
RUN apt-get update \
	&& apt-get install -y cmake make git g++ libssl1.1 libssl-dev \
	&& git clone --recursive --depth 1 --branch boost-1.79.0 https://github.com/boostorg/boost.git /tmp/boost \
	&& cd /tmp/boost && ./bootstrap.sh && ./b2 --with-json --with-system install \
	&& cd /tmp && rm -rf /tmp/boost \
	&& apt-get remove -y git \
	&& rm -rf /var/lib/apt/lists/*
