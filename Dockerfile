FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install required packages including locales
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libboost-all-dev \
    libreadline-dev \
    locales \
    build-essential \
    git \
    curl \
    vim \
    cmake \
    unzip \
    pkg-config \
    libssl-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libcurl4-openssl-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# Generate and configure the locale
RUN locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

WORKDIR /app

COPY . .

RUN make clean && make

CMD ["./bin/HackyDb"]
