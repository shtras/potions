FROM ubuntu:18.04

RUN apt-get update && apt-get -y upgrade
RUN apt-get install -y build-essential cmake git python libboost-system-dev libssl-dev

WORKDIR /opt
RUN git clone https://github.com/mongodb/mongo-c-driver && git clone https://github.com/mongodb/mongo-cxx-driver

RUN mkdir -p /opt/mongo-c-driver/cmake_build /opt/mongo-cxx-driver/cmake_build
WORKDIR /opt/mongo-c-driver/cmake_build
RUN cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF .. && make install -j$(nproc)

WORKDIR /opt/mongo-cxx-driver/cmake_build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. && make EP_mnmlstc_core && make -j$(nproc) && make install
WORKDIR /opt
RUN rm -rf mongo-c-driver mongo-cxx-driver