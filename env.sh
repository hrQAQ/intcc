#!/bin/zsh
conda activate py27
if [ "$1" == "debug" ]; then
    echo "debug"
    CXXFLAGS="-Wno-error -std=c++11" CC='gcc-5' CXX='g++-5' ./waf configure --build-profile=debug
elif [ "$1" == "opt" ]; then
    echo "optimized"
    CXXFLAGS="-Wno-error -std=c++11" CC='gcc-5' CXX='g++-5' ./waf configure --build-profile=optimized
fi