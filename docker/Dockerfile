FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake pkg-config libpugixml-dev libcurl4-openssl-dev rapidjson-dev \
    ca-certificates git && rm -rf /var/lib/apt/lists/*

WORKDIR /work
COPY . /work
RUN cmake -S /work -B /work/build && cmake --build /work/build -j$(nproc)

CMD ["/bin/bash"]
