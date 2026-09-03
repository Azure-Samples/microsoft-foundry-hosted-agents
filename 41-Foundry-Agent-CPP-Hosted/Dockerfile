# syntax=docker/dockerfile:1

FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        git \
        libssl-dev \
        ninja-build \
        pkg-config \
        tar \
        unzip \
        zip \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/microsoft/vcpkg /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics
ENV VCPKG_ROOT=/opt/vcpkg

WORKDIR /src
COPY 40-Foundry-Agent-CPP/ 40-Foundry-Agent-CPP/
COPY 41-Foundry-Agent-CPP-Hosted/ 41-Foundry-Agent-CPP-Hosted/

WORKDIR /src/41-Foundry-Agent-CPP-Hosted
RUN cmake -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DMAF_CPP41_BUILD_TESTS=OFF \
        -B build/release \
    && cmake --build build/release --target maf_agent_cpp_41

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        libcurl4t64 \
        libgcc-s1 \
        libssl3t64 \
        libstdc++6 \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --create-home --uid 10001 agent

COPY --from=build /src/41-Foundry-Agent-CPP-Hosted/build/release/maf_agent_cpp_41 /app/agent

USER agent
WORKDIR /app
EXPOSE 8088

ENTRYPOINT ["/app/agent"]
