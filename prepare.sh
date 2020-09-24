#!/bin/bash
echo "Minidecaf 2018011025"
apt update
apt install -y g++
mkdir build
cd src
g++ montLexer.cpp montParser.cpp montConceiver.cpp montAssembler.cpp montCompiler.cpp -o ../build/MiniDecaf
