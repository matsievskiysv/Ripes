FROM debian:bookworm-backports

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update -q \
    && apt-get install -qy --no-install-recommends \
    build-essential \
    cmake \
    gcc-riscv64-unknown-elf \
    git \
    libpthread-stubs0-dev \
    libqt5charts5-dev \
    libqt5svg5-dev \
    python3 \
    && apt-get autoremove \
    && apt-get autoclean \
    && rm -rf /var/lib/apt/lists/*

ARG GIT_SSL_NO_VERIFY=true
ENV LC_ALL=C.UTF-8 SHELL=/bin/bash

ARG BRANCH=master
RUN git clone --recursive --branch ${BRANCH} https://github.com/mortbopet/Ripes.git /tmp/ripes \
    && cmake -S /tmp/ripes/ -B /tmp/ripes/build -Wno-dev \
    && cmake --build /tmp/ripes/build \
    && cd /tmp/ripes/build && make install \
    && cd /tmp \
    && rm -rf /tmp/ripes

ENTRYPOINT ["Ripes"]
