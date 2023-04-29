FROM registry.gitlab.steamos.cloud/steamrt/scout/sdk:latest

RUN mkdir /home

RUN useradd -m build -u 1000 -U -s /bin/bash

USER build

RUN mkdir -p /home/build/sdl-bits/build

WORKDIR /home/build/sdl-bits/build

COPY --chown=build:build . /home/build/sdl-bits/

RUN cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON ../.

RUN make -j$(nproc)

WORKDIR /home/build/sdl-bits
