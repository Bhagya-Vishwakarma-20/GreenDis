FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY cpp-redis-engine /app/cpp-redis-engine

WORKDIR /app/cpp-redis-engine
RUN rm -rf build CMakeCache.txt CMakeFiles \
    && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target cpp-redis-engine -j"$(nproc)"

EXPOSE 6379

CMD ["./build/cpp-redis-engine", "config/server_config.json"]
