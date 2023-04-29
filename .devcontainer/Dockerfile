FROM mcr.microsoft.com/devcontainers/base:debian

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq && \
    apt-get install -yqq \
    build-essential gdb valgrind cmake ninja-build \
    libdrm-dev libgbm-dev \
    libsdl2-dev libfreetype-dev
