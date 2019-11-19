FROM ubuntu:18.04

RUN apt-get update && apt-get -y upgrade
RUN apt-get install -y build-essential cmake git python libboost-system-dev libssl-dev
RUN mkdir -p /opt/src/
WORKDIR /opt/src
RUN git clone https://github.com/mongodb/mongo-c-driver && git clone https://github.com/mongodb/mongo-cxx-driver
RUN mkdir -p /opt/src/mongo-c-driver/cmake_build /opt/src/mongo-cxx-driver/cmake_build
WORKDIR /opt/src/mongo-c-driver/cmake_build
RUN cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_INSTALL_PREFIX=/opt/mongo-c-driver .. && make install -j$(nproc)

WORKDIR /opt/src/mongo-cxx-driver/cmake_build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/mongo-cxx-driver -DCMAKE_PREFIX_PATH=/opt/mongo-c-driver .. && make EP_mnmlstc_core && make -j$(nproc) && make install
RUN rm -rf /opt/src