FROM mcr.microsoft.com/devcontainers/base:debian

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq && \
    apt-get install -yqq \
    build-essential clang gdb valgrind cmake ninja-build python3-pip \
    libdrm-dev libgbm-dev \
    libsdl2-dev libfreetype-dev
