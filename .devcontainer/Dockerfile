FROM mcr.microsoft.com/devcontainers/base:debian

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq && \
    apt-get install -yqq \
    build-essential clang clangd clang-tidy cmake gcovr gdb meson ninja-build python3-pip valgrind \
    libdrm-dev libgbm-dev \
    libsdl2-dev libfreetype-dev liblua5.4-dev
