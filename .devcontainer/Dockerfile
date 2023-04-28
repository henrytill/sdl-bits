FROM mcr.microsoft.com/devcontainers/base:jammy

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq && \
    apt-get install -yqq \
        build-essential gdb valgrind cmake ninja-build \
        libdrm-dev libgbm-dev \
        libx11-dev libxft-dev libxext-dev \
        libwayland-dev libxkbcommon-dev libegl1-mesa-dev \
        libibus-1.0-dev \
        libasound2-dev libpulse-dev
