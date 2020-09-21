#!/bin/bash
echo "这里面放你在运行前要跑的命令，比如装额外的依赖"
apt update
apt install -y cmake g++ pkg-config uuid-dev
mkdir build
cd build
cmake ..
make
cd ..
